// proc_simplefile.cpp
//
// Simple File Processor
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

#include <unistd.h>
#include <sys/types.h>

#include "proc_simplefile.h"

ProcSimpleFile::ProcSimpleFile(SyProfile *c,int recv_num,int proc_num,QObject *parent)
  : Processor(c,recv_num,proc_num,parent)
{
  bool ok=false;
  
  d_base_filename=logRootDirectory()->path()+"/"+
    config()->stringValue(QString::asprintf("Receiver%d",1+receiverNumber()),
			  QString::asprintf("Processor%dBaseFilename",
					    1+processorNumber()),"",&ok);
  if(!ok) {
    fprintf(stderr,"lwsyslogger: missing BaseFilename in Receiver%d:%d\n",
	    1+receiverNumber(),1+processorNumber());
    exit(1);
  }
  d_base_file=NULL;
}


Processor::Type ProcSimpleFile::type() const
{
  return Processor::TypeSimpleFile;
}


void ProcSimpleFile::closeFiles()
{
  if(d_base_file!=NULL) {
    fclose(d_base_file);
    d_base_file=NULL;
  }
}


void ProcSimpleFile::processMessage(Message *msg,const QHostAddress &from_addr)
{
  if(d_base_file==NULL) {
    d_base_file=fopen(d_base_filename.toUtf8(),"a");
  }
  if(d_base_file!=NULL) {
    fprintf(d_base_file,"%s %s %s\n",
	    msg->timestamp().toString("MMM dd hh:mm:ss").toUtf8().constData(),
	    msg->hostName().toUtf8().constData(),
	    msg->msg().toUtf8().constData());
    fflush(d_base_file);
  }
}
