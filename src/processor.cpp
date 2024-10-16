// processor.cpp
//
// Abstract base class for message processors.
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

#include "processor.h"

Processor::Processor(SyProfile *c,int recv_num,int proc_num,QObject *parent)
  : QObject(parent)
{
  bool ok=false;
  
  d_config=c;
  d_receiver_number=recv_num;
  d_processor_number=proc_num;
  d_log_root_directory=new QDir(c->stringValue("Global","LogRoot","",&ok));
  if(!ok) {
    fprintf(stderr,"lwsyslogger: unable to load location of \"LogRoot\"\n");
    exit(1);
  }
}


bool Processor::start(QString *err_msg)
{
  return true;
}


void Processor::closeFiles()
{
}


SyProfile *Processor::config() const
{
  return d_config;
}


int Processor::receiverNumber() const
{
  return d_receiver_number;
}


int Processor::processorNumber() const
{
  return d_processor_number;
}


QString Processor::typeString(Processor::Type type)
{
  QString ret="UNKNOWN";
  
  switch(type) {
  case Processor::TypeSimpleFile:
    ret="SimpleFile";
    break;

  case Processor::TypeLast:
    break;
  }

  return ret;
}


Processor::Type Processor::typeFromString(const QString &str)
{
  for(int i=0;i<Processor::TypeLast;i++) {
    if(Processor::typeString((Processor::Type)i).toLower()==str.toLower()) {
      return (Processor::Type)i;
    }
  }
  
  return Processor::TypeLast;
}


QDir *Processor::logRootDirectory() const
{
  return d_log_root_directory;
}
