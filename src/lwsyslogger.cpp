// lwsyslogger.cpp
//
// Simple syslog logger for Livewire sites
//
//   (C) Copyright 2024 Fred Gleason <fredg@paravelsystems.com>
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License version 2 as
//   published by the Free Software Foundation.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public
//   License along with this program; if not, write to the Free Software
//   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//

#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <QCoreApplication>

#include "cmdswitch.h"
#include "lwsyslogger.h"
#include "recv_factory.h"

//
// Globals
//
bool no_local_syslog=false;
bool global_exiting=false;
QList<Receiver *> *syslog_receivers=NULL;

void SigHandler(int signo)
{
  switch(signo) {
  case SIGINT:
  case SIGTERM:
    global_exiting=true;
    break;
  }
}

bool debug=false;
void LocalSyslog(Message::Severity severity,const char *fmt,...)
{
  static char buffer[1024];
  static QList<Message::Severity> early_severities;
  static QStringList early_msgs;
  static Message *msg=NULL;

  if(!no_local_syslog) {
    va_list args;
    va_start(args,fmt);
    if(vsnprintf(buffer,1024,fmt,args)>0) {
      if(debug) {
	fprintf(stderr,"%s\n",buffer);
      }
      if(syslog_receivers==NULL) {
	early_severities.push_back(severity);
	early_msgs.push_back(buffer);
      }
      else {
	for(int i=0;i<early_msgs.size();i++) {
	  msg=new Message(early_severities.at(i),early_msgs.at(i));
	  for(int j=0;j<syslog_receivers->size();j++) {
	    syslog_receivers->at(j)->
	      processMessage(msg,QHostAddress("127.0.0.1"));
	  }
	  delete msg;
	}
	early_msgs.clear();
	early_severities.clear();
	msg=new Message(severity,buffer);
	for(int i=0;i<syslog_receivers->size();i++) {
	  syslog_receivers->at(i)->
	    processMessage(msg,QHostAddress("127.0.0.1"));
	}
	delete msg;
      }
    }
    va_end(args);
  }
}


MainObject::MainObject(QObject *parent)
  : QObject(parent)
{
  QString config_filename=DEFAULT_CONFIG_FILENAME;
  QDateTime rotate_logfiles_datetime=QDateTime::currentDateTime();
  bool rotate_logfiles=false;

  //
  // Read Switches
  //
  CmdSwitch *cmd=new CmdSwitch("lwsyslogger",VERSION,LWSYSLOGGER_USAGE);
  for(int i=0;i<cmd->keys();i++) {
    if(cmd->key(i)=="--config") {
      config_filename=cmd->value(i);
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="-d") {
      debug=true;
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="--no-local-syslog") {
      no_local_syslog=true;
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="--rotate-logfiles") {
      rotate_logfiles=true;
      if(!cmd->value(i).isEmpty()) {
	rotate_logfiles_datetime=
	  QDateTime::fromString(cmd->value(i),"yyyy-MM-ddThh:mm:ss");
	if(!rotate_logfiles_datetime.isValid()) {
	  fprintf(stderr,
	    "lwsyslogger: invalid date-time specified for --rotate-logfiles\n");
	  exit(1);
	}
      }
      cmd->setProcessed(i,true);
    }
    if(!cmd->processed(i)) {
      fprintf(stderr,"lwsyslogger: unrecognized switch \"%s\"\n",
	      cmd->key(i).toUtf8().constData());
      exit(1);
    }
  }

  //
  // Verify that the LogRoot is configured correctly
  //
  d_config=new Profile();
  if(!d_config->loadFile(config_filename)) {
    fprintf(stderr,"lwsyslogger: cannot open configuration file \"%s\"\n",
	    config_filename.toUtf8().constData());
    exit(1);
  }
  QString logroot=d_config->stringValue("Global","LogRoot",DEFAULT_LOGROOT);
  QString service_user=
    d_config->stringValue("Global","ServiceUser",DEFAULT_SERVICE_USER);
  struct passwd *passwd=getpwnam(service_user.toUtf8());
  if(passwd==NULL) {
    fprintf(stderr,
	    "lwsyslogger: cannot find ServiceUser \"%s\" [No such user]\n",
	    service_user.toUtf8().constData());
    exit(1);
  }
  if(passwd->pw_uid==0) {
    fprintf(stderr,"lwsyslogger: running under UID \"0\" is not supported\n");
    exit(1);
  }
  d_uid=passwd->pw_uid;

  QString service_group=
    d_config->stringValue("Global","ServiceGroup",DEFAULT_SERVICE_GROUP);
  struct group *group=getgrnam(service_group.toUtf8());
  if(group==NULL) {
    fprintf(stderr,
	    "lwsyslogger: cannot find ServiceGroup \"%s\" [No such group]\n",
	    service_group.toUtf8().constData());
    exit(1);
  }
  d_gid=group->gr_gid;
  
  struct stat statbuf;
  memset(&statbuf,0,sizeof(statbuf));
  if(stat(logroot.toUtf8(),&statbuf)!=0) {
    fprintf(stderr,
	    "lwsyslogger: unable to stat LogRoot directory \"%s\" [%s]\n",
	    logroot.toUtf8().constData(),strerror(errno));
    exit(1);
  }
  if(statbuf.st_uid!=d_uid) {
    fprintf(stderr,
	    "lwsyslogger: LogRoot directory \"%s\" not owned by user \"%s\"\n",
	    logroot.toUtf8().constData(),service_user.toUtf8().constData());
    exit(1);
  }
  if(statbuf.st_gid!=d_gid) {
    fprintf(stderr,
	    "lwsyslogger: LogRoot directory \"%s\" not owned by group \"%s\"\n",
	    logroot.toUtf8().constData(),service_group.toUtf8().constData());
    exit(1);
  }
  if((statbuf.st_mode&S_IRUSR)==0) {
    fprintf(stderr,
	    "lwsyslogger: LogRoot directory \"%s\" is not readable by user \"%s\"\n",
	    logroot.toUtf8().constData(),service_user.toUtf8().constData());
    exit(1);
  }
  if((statbuf.st_mode&S_IWUSR)==0) {
    fprintf(stderr,
	    "lwsyslogger: LogRoot directory \"%s\" is not writeable by user \"%s\"\n",
	    logroot.toUtf8().constData(),service_user.toUtf8().constData());
    exit(1);
  }

  d_logroot_dir=new QDir(logroot);
  
  //
  // Start Receivers
  //
  Receiver *recv=NULL;
  QString err_msg;
  bool ok=false;
  int count=0;
  QString section=QString::asprintf("Receiver%d",1+count);
  Receiver::Type type=
    Receiver::typeFromString(d_config->stringValue(section,"Type",0,&ok));
  while(ok) {
    if(type==Receiver::TypeLast) {
      fprintf(stderr,"lwsyslogger: unknown receiver type \"%s\"\n",
	      d_config->stringValue(section,"Type").toUtf8().constData());
      exit(1);
    }
    if((recv=ReceiverFactory(type,d_config,count,this))==NULL) {
      fprintf(stderr,"lwsyslogger: failed to initialize receiver type \"%s\"\n",
	      Receiver::typeString(type).toUtf8().constData());
      exit(1);
    }
    if(!recv->start(&err_msg)) {
      fprintf(stderr,"lwsyslogger: failed to start %s [%s]\n",
	      section.toUtf8().constData(),err_msg.toUtf8().constData());
      exit(1);
    }
    d_receivers.push_back(recv);
    
    count++;
    section=QString::asprintf("Receiver%d",1+count);
    type=(Receiver::Type)d_config->intValue(section,"Port",0,&ok);
  }
  syslog_receivers=&d_receivers;

  //
  // Drop Root Permissions
  //
  if(setgid(d_gid)!=0) {
    fprintf(stderr,"lwsyslogger: failed to set GID for \"%s\" [%s]\n",
	    service_group.toUtf8().constData(),strerror(errno));
    exit(1);
  }
  if(setuid(d_uid)!=0) {
    fprintf(stderr,"lwsyslogger: failed to set UID for \"%s\" [%s]\n",
	    service_user.toUtf8().constData(),strerror(errno));
    exit(1);
  }

  //
  // Exit Timer
  //
  d_exit_timer=new QTimer(this);
  d_exit_timer->setSingleShot(false);
  connect(d_exit_timer,SIGNAL(timeout()),this,SLOT(exitData()));
  d_exit_timer->start(500);
  signal(SIGINT,SigHandler);
  signal(SIGTERM,SigHandler);

  LocalSyslog(Message::SeverityNotice,"lwsyslogger v%s started",VERSION);

  //
  // Force Log Rotation
  //
  if(rotate_logfiles) {
    for(int i=0;i<d_receivers.size();i++) {
      d_receivers.at(i)->rotateLogs(rotate_logfiles_datetime);
    }
    global_exiting=true;
  }
}


void MainObject::exitData()
{
  if(global_exiting) {
    LocalSyslog(Message::SeverityNotice,"lwsyslogger v%s exiting",VERSION);
    exit(0);
  }
}


int main(int argc,char *argv[])
{
  QCoreApplication a(argc,argv);

  new MainObject();

  return a.exec();
}
