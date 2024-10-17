// receiver.cpp
//
// Abstract base class for transport layer receivers.
//
// For basic concepts regarding  Syslog Transport Layer Protocols
// see RFC 5424 Section 5)
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

#include "proc_factory.h"
#include "receiver.h"

Receiver::Receiver(SyProfile *c,int recv_num,QObject *parent)
  : QObject(parent)
{
  d_config=c;
  d_receiver_number=recv_num;

  //
  // Create Processor(s)
  //
  bool ok=false;
  int count=0;
  QString section=QString::asprintf("Receiver%d",1+recv_num);
  QString param=QString::asprintf("Processor%dType",1+count);
  Processor::Type type=
    Processor::typeFromString(d_config->stringValue(section,param,0,&ok));
  while(ok) {
    if(type==Processor::TypeLast) {
      exit(1);
    }
    addProcessor(type,count);
    count++;
    param=QString::asprintf("Processor%dType",1+count);
    type=Processor::typeFromString(d_config->stringValue(section,param,0,&ok));
  }
}


void Receiver::closeFiles()
{
  for(QMap<int,Processor *>::const_iterator it=d_processors.begin();
      it!=d_processors.end();it++) {
    it.value()->closeFiles();
  }
}


void Receiver::addProcessor(Processor::Type type,int proc_num)
{
  QString err_msg;
  Processor *proc=
    ProcessorFactory(type,config(),receiverNumber(),proc_num,this);
  if(proc==NULL) {
    fprintf(stderr,"lwsyslogger: failed to initialize processor type \"%s\"\n",
	      Processor::typeString(type).toUtf8().constData());
    exit(1);
  }
  if(!proc->start(&err_msg)) {
    fprintf(stderr,
	    "lwsyslogger: failed to start Receiver%d:%d processor [%s]\n",
	    receiverNumber(),proc_num,err_msg.toUtf8().constData());
    exit(1);
  }
  d_processors[proc_num]=proc;
}


void Receiver::processMessage(Message *msg,const QHostAddress &from_addr)
{
  //  emit messageReceived(msg,from_addr);
  for(QMap<int,Processor *>::const_iterator it=d_processors.begin();
      it!=d_processors.end();it++) {
    it.value()->process(msg,from_addr);
  }
}


SyProfile *Receiver::config() const
{
  return d_config;
}


int Receiver::receiverNumber() const
{
  return d_receiver_number;
}


QString Receiver::typeString(Receiver::Type type)
{
  QString ret="UNKNOWN";
  
  switch(type) {
  case Receiver::TypeUdp:
    ret="UDP";
    break;

  case Receiver::TypeLast:
    break;
  }

  return ret;
}


Receiver::Type Receiver::typeFromString(const QString &str)
{
  for(int i=0;i<Receiver::TypeLast;i++) {
    if(Receiver::typeString((Receiver::Type)i).toLower()==str.toLower()) {
      return (Receiver::Type)i;
    }
  }
  
  return Receiver::TypeLast;
}
