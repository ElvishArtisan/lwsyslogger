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
#include "proc_factory.h"
#include "recv_factory.h"

//
// Globals
//
bool no_local_syslog=false;
bool global_exiting=false;
QMap<QString,Processor *> *syslog_processors=NULL;

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
      if(syslog_processors==NULL) {
	early_severities.push_back(severity);
	early_msgs.push_back(buffer);
      }
      else {
	for(int i=0;i<early_msgs.size();i++) {
	  msg=new Message(early_severities.at(i),early_msgs.at(i));
	  for(QMap<QString,Processor *>::const_iterator it=
		syslog_processors->begin();it!=syslog_processors->end();it++) {
	    it.value()->process(msg,QHostAddress("127.0.0.1"));
	  }
	  delete msg;
	}
	early_msgs.clear();
	early_severities.clear();

	msg=new Message(severity,buffer);
	for(QMap<QString,Processor *>::const_iterator it=
	      syslog_processors->begin();it!=syslog_processors->end();it++) {
	  it.value()->process(msg,QHostAddress("127.0.0.1"));
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
  bool dry_run=false;
  bool dump_config=false;
  QString err_msg;
  QStringList err_msgs;
  
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
    if(cmd->key(i)=="--dry-run") {
      dry_run=true;
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="--dump-config") {
      dump_config=true;
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
  // Create Configuration Context
  //
  d_profile=new Profile(true);
  if(d_profile->load(config_filename,&err_msgs)<=0) {
    fprintf(stderr,"lwsyslogger: cannot open configuration file \"%s\" [%s]\n",
	    config_filename.toUtf8().constData(),err_msg.toUtf8().constData());
    exit(1);
  }
  QStringList config_dirs=
    d_profile->stringValues("Global","Default","IncludeConfig");
  for(int i=0;i<config_dirs.size();i++) {
    int num=d_profile->load(config_dirs.at(i),&err_msgs);
    if(num<0) {
      fprintf(stderr,
	 "lwsyslogger: failed attempting to read configuration from \"%s\":\n",
	      config_dirs.at(i).toUtf8().constData());
      for(int j=0;j<err_msgs.size();j++) {
	fprintf(stderr,"    error[%d]: %s\n",1+j,
		err_msgs.at(i).toUtf8().constData());
      }
      exit(1);
    }
    LocalSyslog(Message::SeverityDebug,"loaded %d configurations from \"%s\"",
		num,config_dirs.at(i).toUtf8().constData());
  }
  if(dump_config) {
    for(int i=0;i<err_msgs.size();i++) {
      fprintf(stderr,"%s\n",err_msgs.at(i).toUtf8().constData());
    }
    printf("%s\n",d_profile->dump().toUtf8().constData());
    exit(0);
  }
  
  //
  // Verify that the LogRoot is configured correctly
  //
  if(!ConfigureLogRoot(&err_msg)) {
    fprintf(stderr,"lwsyslogger: %s\n",err_msg.toUtf8().constData());
    exit(1);
  }

  //
  // Start Processors
  //
  if(!StartProcessors(&err_msg,dry_run)) {
    fprintf(stderr,"lwsyslogger: %s\n",err_msg.toUtf8().constData());
    exit(1);
  }

  //
  // Start Receivers
  //
  if(!StartReceivers(&err_msg)) {
    fprintf(stderr,"lwsyslogger: %s\n",err_msg.toUtf8().constData());
    exit(1);
  }

  //
  // Drop Root Permissions
  //
  if(setgid(d_gid)!=0) {
    fprintf(stderr,"lwsyslogger: failed to set GID for \"%s\" [%s]\n",
	    d_group_name.toUtf8().constData(),strerror(errno));
    exit(1);
  }
  if(setuid(d_uid)!=0) {
    fprintf(stderr,"lwsyslogger: failed to set UID for \"%s\" [%s]\n",
	    d_user_name.toUtf8().constData(),strerror(errno));
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
    for(QMap<QString,Processor *>::const_iterator it=d_processors.begin();
	it!=d_processors.end();it++) {
      it.value()->rotateLogs(rotate_logfiles_datetime);
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


bool MainObject::ConfigureLogRoot(QString *err_msg)
{
  QStringList values=d_profile->stringValues("Global","Default","LogRoot");
  QString logroot=DEFAULT_LOGROOT;
  if(!values.isEmpty()) {
    logroot=values.last();
  }

  values=d_profile->stringValues("Global","Default","ServiceUser");
  d_user_name=DEFAULT_SERVICE_USER;
  if(!values.isEmpty()) {
    d_user_name=values.last();
  }

  struct passwd *passwd=getpwnam(d_user_name.toUtf8());
  if(passwd==NULL) {
    *err_msg=QString::asprintf("cannot find ServiceUser \"%s\" [No such user]",
			       d_user_name.toUtf8().constData());
    return false;
  }
  if(passwd->pw_uid==0) {
    *err_msg=QString::asprintf("running under UID \"0\" is not supported");
    return false;
  }
  d_uid=passwd->pw_uid;

  values=d_profile->stringValues("Global","Default","ServiceGroup");
  d_group_name=DEFAULT_SERVICE_GROUP;
  if(!values.isEmpty()) {
    d_group_name=values.last();
  }

  struct group *group=getgrnam(d_group_name.toUtf8());
  if(group==NULL) {
    *err_msg=QString::asprintf(
	    "lwsyslogger: cannot find ServiceGroup \"%s\" [No such group]",
	    d_group_name.toUtf8().constData());
    return false;
  }
  d_gid=group->gr_gid;

  struct stat statbuf;
  memset(&statbuf,0,sizeof(statbuf));
  if(stat(logroot.toUtf8(),&statbuf)!=0) {
    *err_msg=QString::asprintf(
	    "lwsyslogger: unable to stat LogRoot directory \"%s\" [%s]",
	    logroot.toUtf8().constData(),strerror(errno));
    return false;
  }
  if(statbuf.st_uid!=d_uid) {
    *err_msg=QString::asprintf(
	    "lwsyslogger: LogRoot directory \"%s\" not owned by user \"%s\"",
	    logroot.toUtf8().constData(),d_user_name.toUtf8().constData());
    return false;
  }
  if(statbuf.st_gid!=d_gid) {
    *err_msg=QString::asprintf(
	    "lwsyslogger: LogRoot directory \"%s\" not owned by group \"%s\"",
	    logroot.toUtf8().constData(),d_group_name.toUtf8().constData());
    return false;
  }
  if((statbuf.st_mode&S_IRUSR)==0) {
    *err_msg=QString::asprintf(
      "lwsyslogger: LogRoot directory \"%s\" is not readable by user \"%s\"",
	    logroot.toUtf8().constData(),d_user_name.toUtf8().constData());
    return false;
  }
  if((statbuf.st_mode&S_IWUSR)==0) {
    *err_msg=QString::asprintf(
      "lwsyslogger: LogRoot directory \"%s\" is not writeable by user \"%s\"",
	    logroot.toUtf8().constData(),d_user_name.toUtf8().constData());
    return false;
  }

  d_logroot_dir=new QDir(logroot);

  return true;
}


bool MainObject::StartProcessors(QString *err_msg,bool dry_run)
{
  QString err_msg2;
  Processor *proc=NULL;
  QStringList ids=d_profile->sectionIds("Processor");
  for(int i=0;i<ids.size();i++) {
    QStringList values=d_profile->stringValues("Processor",ids.at(i),"Type");
    if(values.isEmpty()) {
      *err_msg=QString::asprintf("%s: no processor type specified",
				 ids.at(i).toUtf8().constData());
      return false;
    }
    QString procstr=
      d_profile->stringValues("Processor",ids.at(i),"Type").last();
    Processor::Type type=Processor::typeFromString(values.last());
    if(type==Processor::TypeLast) {
      *err_msg=QString::asprintf("%s: unknown processor type \"%s\"",
				 ids.at(i).toUtf8().constData(),
				 values.last().toUtf8().constData());
      return false;
    }
    if((proc=ProcessorFactory(type,ids.at(i),d_profile,this))==NULL) {
      *err_msg=QString::asprintf(
	      "%s: failed to initialize processor type \"%s\"",
	      ids.at(i).toUtf8().constData(),
	      Processor::typeString(type).toUtf8().constData());
      return false;
    }
    proc->setDryRun(dry_run);
    if(!proc->start(&err_msg2)) {
      *err_msg=QString::asprintf("%s: failed to start processor [%s]",
				 ids.at(i).toUtf8().constData(),
				 err_msg2.toUtf8().constData());
      return false;
    }
    d_processors[ids.at(i)]=proc;
  }
  
  return true;
}


bool MainObject::StartReceivers(QString *err_msg)
{
  QStringList values;
  Receiver *recv=NULL;
  QStringList ids=d_profile->sectionIds("Receiver");

  for(int i=0;i<ids.size();i++) {
    values=d_profile->stringValues("Receiver",ids.at(i),"Type");
    if(values.isEmpty()) {
      *err_msg=QString::asprintf("%s: no receiver type specified",
				 ids.at(i).toUtf8().constData());
      return false;
    }
    Receiver::Type type=Receiver::typeFromString(values.last());
    if(type==Receiver::TypeLast) {
      *err_msg=QString::asprintf("%s: unknown receiver type \"%s\"",
				 ids.at(i).toUtf8().constData(),
				 values.last().toUtf8().constData());
      return false;
    }
    if((recv=ReceiverFactory(type,ids.at(i),d_profile,this))==NULL) {
      *err_msg=QString::asprintf(
	   "%s: failed to initialize receiver type \"%s\"",
	   ids.at(i).toUtf8().constData(),
	   Receiver::typeString(type).toUtf8().constData());
      return false;
    }
    QString err_msg2;
    if(!recv->start(&err_msg2)) {
      *err_msg=QString::asprintf("%s: failed to start receiver [%s]",
				 ids.at(i).toUtf8().constData(),
				 err_msg2.toUtf8().constData());
      return false;
    }
    d_receivers[ids.at(i)]=recv;
    QStringList processors=
      d_profile->stringValues("Receiver",ids.at(i),"Processor");
    for(int j=0;j<processors.size();j++) {
      Processor *proc=d_processors.value(processors.at(j));
      if(proc==NULL) {
	*err_msg=QString::asprintf(
	     "unknown processor \"%s\" specified in receiver \"%s\"",
	     processors.at(j).toUtf8().constData(),
	     ids.at(i).toUtf8().constData());
	return false;
      }
      connect(recv,SIGNAL(messageReceived(Message *,const QHostAddress &)),
	      proc,SLOT(process(Message *,const QHostAddress &)));
    }
  }
  syslog_processors=&d_processors;

  return true;
}


int main(int argc,char *argv[])
{
  QCoreApplication a(argc,argv);

  new MainObject();

  return a.exec();
}
