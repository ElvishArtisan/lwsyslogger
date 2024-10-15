// lwlogger.cpp
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

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

#include <QCoreApplication>

#include <sy5/sycmdswitch.h>

#include "lwlogger.h"
#include "recv_factory.h"

MainObject::MainObject(QObject *parent)
  : QObject(parent)
{
  bool debug=false;
  QString config_filename=DEFAULT_CONFIG_FILENAME;
  
  //
  // Read Switches
  //
  SyCmdSwitch *cmd=new SyCmdSwitch("lwlogger",VERSION,LWLOGGER_USAGE);
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
      fprintf(stderr,"lwlogger: unrecognized switch \"%s\"\n",
	      cmd->key(i).toUtf8().constData());
      exit(1);
    }
  }

  //
  // Configuration
  //
  d_config=new SyProfile();
  if(!d_config->setSource(config_filename)) {
    fprintf(stderr,"lwlogger: cannot open configuration file \"%s\"\n",
	    config_filename.toUtf8().constData());
    exit(1);
  }

  //
  // Syslog
  //
  if(debug) {
    openlog("lwlogger",LOG_PERROR,LOG_SYSLOG);
  }
  else {
    openlog("lwlogger",0,LOG_SYSLOG);
  }
  
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
      fprintf(stderr,"lwlogger: unknown receiver type \"%s\"\n",
	      d_config->stringValue(section,"Type").toUtf8().constData());
      exit(1);
    }
    if((recv=ReceiverFactory(type,d_config,count,this))==NULL) {
      fprintf(stderr,"lwlogger: failed to initialize receiver type \"%s\"\n",
	      Receiver::typeString(type).toUtf8().constData());
      exit(1);
    }
    connect(recv,SIGNAL(messageReceived(Message *,const QHostAddress &)),
	    this,SLOT(messageReceivedData(Message *,const QHostAddress &)));
    if(!recv->start(&err_msg)) {
      fprintf(stderr,"lwlogger: failed to start %s [%s]\n",
	      section.toUtf8().constData(),err_msg.toUtf8().constData());
      exit(1);
    }
    d_receivers.push_back(recv);
    
    count++;
    section=QString::asprintf("Receiver%d",1+count);
    type=(Receiver::Type)d_config->intValue(section,"Port",0,&ok);
  }
}


void MainObject::messageReceivedData(Message *msg,const QHostAddress &from_addr)
{
  printf("******************************************\n");
  printf("%s\n",msg->dump().toUtf8().constData());
}


int main(int argc,char *argv[])
{
  QCoreApplication a(argc,argv);

  new MainObject();

  return a.exec();
}
