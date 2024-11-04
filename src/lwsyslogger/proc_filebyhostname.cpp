// proc_filebyhostname.cpp
//
// Processor for logging to files based on the hostname.
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

#include "proc_filebyhostname.h"

ProcFileByHostname::ProcFileByHostname(const QString &id,Profile *p,
				       QObject *parent)
  : Processor(id,p,parent)
{
  QStringList values=p->stringValues("Processor",id,"BaseDirname");
  if(values.isEmpty()) {
    fprintf(stderr,"lwsyslogger: missing BaseDirname in processor %s\n",
	    id.toUtf8().constData());
    exit(1);
  }
  d_base_dir=new QDir(logRootDirectory()->path()+"/"+values.last());
  if(!d_base_dir->exists()) {
    fprintf(stderr,"lwsyslogger: directory \"%s\" does not exist",
	    d_base_dir->path().toUtf8().constData());
    exit(1);
  }
  lsyslog(Message::SeverityDebug,"using base_dir \"%s\"",
	  d_base_dir->path().toUtf8().constData());
}


Processor::Type ProcFileByHostname::type() const
{
  return Processor::TypeFileByHostname;
}


bool ProcFileByHostname::start(QString *err_msg)
{
  return true;
}


void ProcFileByHostname::rotateLogs(const QDateTime &now)
{
  for(QMap<QString,FILE *>::const_iterator it=d_files.begin();it!=d_files.end();
      it++) {
    fclose(it.value());
  }
  d_files.clear();
  QStringList filenames=d_base_dir->entryList(QDir::Files);
  for(int i=0;i<filenames.size();i++) {
    if(filenames.at(i).contains("-")) {
      expireLogFile(d_base_dir->path()+"/"+filenames.at(i),now);
    }
    else {
      rotateLogFile(d_base_dir->path()+"/"+filenames.at(i),now);
    }
  }
}


void ProcFileByHostname::processMessage(Message *msg,
					const QHostAddress &from_addr)
{
  FILE *f=NULL;
  QString hostname=msg->hostName();
  if(msg->hostName().isEmpty()) {
    hostname=from_addr.toString();
    hostname.replace(".","_");
    lsyslog(Message::SeverityWarning,"message from %s has empty hostname",
	    from_addr.toString().toUtf8().constData());
    return;
  }
  hostname.replace("-","_");
  QString pathname=d_base_dir->path()+"/"+hostname;
  if((f=d_files.value(pathname))==NULL) {
    if((f=fopen(pathname.toUtf8(),"w"))==NULL) {
      lsyslog(Message::SeverityWarning,"failed to open file \"%s\" [%s]",
	      pathname.toUtf8().constData(),strerror(errno));
      return;
    }
    d_files[pathname]=f;
  }
  fprintf(f,"%s\n",msg->resolveWildcards(messageTemplate()).toUtf8().constData());
  fflush(f);
}
