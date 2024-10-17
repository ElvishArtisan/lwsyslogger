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
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <QCoreApplication>

#include <sy5/sycmdswitch.h>

#include "lwsyslogger.h"
#include "recv_factory.h"

MainObject::MainObject(QObject *parent)
  : QObject(parent)
{
  bool debug=false;
  QString config_filename=DEFAULT_CONFIG_FILENAME;
  
  //
  // Read Switches
  //
  SyCmdSwitch *cmd=new SyCmdSwitch("lwsyslogger",VERSION,LWSYSLOGGER_USAGE);
  for(int i=0;i<cmd->keys();i++) {
    if(cmd->key(i)=="--config") {
      config_filename=cmd->value(i);
      cmd->setProcessed(i,true);
    }
    if(cmd->key(i)=="-d") {
      debug=true;
      cmd->setProcessed(i,true);
    }
    if(!cmd->processed(i)) {
      fprintf(stderr,"lwsyslogger: unrecognized switch \"%s\"\n",
	      cmd->key(i).toUtf8().constData());
      exit(1);
    }
  }

  //
  // Syslog
  //
  if(debug) {
    openlog("lwsyslogger",LOG_PERROR,LOG_SYSLOG);
  }
  else {
    openlog("lwsyslogger",0,LOG_SYSLOG);
  }
  
  //
  // Verify that the LogRoot is configured correctly
  //
  d_config=new SyProfile();
  if(!d_config->setSource(config_filename)) {
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
    connect(recv,SIGNAL(messageReceived(Message *,const QHostAddress &)),
	    this,SLOT(messageReceivedData(Message *,const QHostAddress &)));
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
}


void MainObject::messageReceivedData(Message *msg,const QHostAddress &from_addr)
{
  //  printf("******************************************\n");
  //  printf("%s",msg->dump().toUtf8().constData());
}


int main(int argc,char *argv[])
{
  QCoreApplication a(argc,argv);

  new MainObject();

  return a.exec();
}
