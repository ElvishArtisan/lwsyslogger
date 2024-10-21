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

#include <errno.h>
#include <unistd.h>
#include <unistd.h>
#include <sys/types.h>

#include "local_syslog.h"
#include "proc_simplefile.h"

ProcSimpleFile::ProcSimpleFile(Profile *c,int recv_num,int proc_num,QObject *parent)
  : Processor(c,recv_num,proc_num,parent)
{
  bool ok=false;
  QString section=QString::asprintf("Receiver%d",1+receiverNumber());
  QString proc_base=QString::asprintf("Processor%d",1+proc_num);
  
  //
  // Get Base Filename and Pathname
  //
  d_base_pathname=logRootDirectory()->path()+"/"+
    config()->stringValue(section,proc_base+"BaseFilename","",&ok);
  if(!ok) {
    fprintf(stderr,"lwsyslogger: missing BaseFilename in %s\n",
	    idString().toUtf8().constData());
    exit(1);
  }
  QStringList f0=d_base_pathname.split("/",Qt::KeepEmptyParts);
  d_base_filename=f0.last();
  f0.removeLast();
  if(f0.size()==0) {
    d_base_dir=logRootDirectory();
  }
  else {
    d_base_dir=new QDir(f0.join("/"));
  }
  LocalSyslog(Message::SeverityDebug,"%s using base_dir \"%s\"",
	      idString().toUtf8().constData(),
	      d_base_dir->path().toUtf8().constData());
  d_base_file=NULL;

  //
  // Get Record Template
  //
  d_template=config()->stringValue(section,proc_base+"Template","",&ok);
  if(!ok) {
    fprintf(stderr,"lwsyslogger: missing Template in %s\n",
	    idString().toUtf8().constData());
    exit(1);
  }
}


Processor::Type ProcSimpleFile::type() const
{
  return Processor::TypeSimpleFile;
}


void ProcSimpleFile::rotateLogs(const QDateTime &now)
{
  //
  // Rotate Base File
  //
  if(d_base_file!=NULL) {
    fclose(d_base_file);
    d_base_file=NULL;
  }
  rotateLogFile(d_base_pathname,now);
  if((d_base_file=fopen(d_base_pathname.toUtf8(),"a"))==NULL) {
    LocalSyslog(Message::SeverityWarning,
		"%s: failed to reopen logfile %s [%s]",
		idString().toUtf8().constData(),
		d_base_pathname.toUtf8().constData(),strerror(errno));
  }

  //
  // Delete Expired Files
  //
  QStringList name_filters;
  name_filters.push_back(d_base_filename+"-*");
  QStringList filenames=d_base_dir->entryList(name_filters,QDir::Files);
  for(int i=0;i<filenames.size();i++) {
    expireLogFile(d_base_dir->path()+"/"+filenames.at(i),now);
  }
}


void ProcSimpleFile::processMessage(Message *msg,const QHostAddress &from_addr)
{
  if(d_base_file==NULL) {
    d_base_file=fopen(d_base_pathname.toUtf8(),"a");
  }
  if(d_base_file!=NULL) {
    fprintf(d_base_file,"%s\n",
	    msg->resolveWildcards(d_template).toUtf8().constData());
    fflush(d_base_file);
  }
}
