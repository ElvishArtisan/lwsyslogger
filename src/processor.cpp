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

#include <stdio.h>
#include <unistd.h>

#include <QFile>
#include <QFileInfo>

#include "local_syslog.h"
#include "processor.h"

Processor::Processor(SyProfile *c,int recv_num,int proc_num,QObject *parent)
  : QObject(parent)
{
  bool ok=false;
  QString err_msg;
  
  d_config=c;
  d_receiver_number=recv_num;
  d_processor_number=proc_num;
  d_log_root_directory=new QDir(c->stringValue("Global","LogRoot","",&ok));
  if(!ok) {
    fprintf(stderr,"lwsyslogger: unable to load location of \"LogRoot\"\n");
    exit(1);
  }

  QString section=QString::asprintf("Receiver%d",1+recv_num);
  QString proc_base=QString::asprintf("Processor%d",1+proc_num);

  //
  // Load Facility/Severity Filter Values
  //
  QString param=c->stringValue(section,proc_base+"Facility",0,&ok);
  if(ok) {
    d_facility_mask=MakeFacilityMask(param,&ok,&err_msg);
  }
  param=c->stringValue(section,proc_base+"Severity",0,&ok);
  if(ok) {
    d_severity_mask=MakeSeverityMask(param,&ok,&err_msg);
  }

  //
  // Load Log Rotation Values
  //
  d_log_rotation_age=
    config()->intValue(section,proc_base+"LogRotationAge");
  d_log_rotation_size=config()->intValue(section,proc_base+"LogRotationSize");
  d_old_log_purge_age=config()->intValue(section,proc_base+"OldLogPurgeAge");
  param=c->stringValue(section,proc_base+"LogRotationTime","00:00");
  d_log_rotation_time=QTime::fromString(param,"hh:mm");
  if(!d_log_rotation_time.isValid()) {
    fprintf(stderr,"invalid time value \"%s\" in %sLogRotationTime\n",
	    param.toUtf8().constData(),proc_base.toUtf8().constData());
    exit(1);
  }
  d_log_rotation_timer=new QTimer(this);
  d_log_rotation_timer->setSingleShot(true);
  connect(d_log_rotation_timer,SIGNAL(timeout()),this,SLOT(logRotationData()));
  StartLogRotationTimer();
}


bool Processor::start(QString *err_msg)
{
  return true;
}


void Processor::process(Message *msg,const QHostAddress &from_addr)
{
  if(processIf(msg->severity())&&processIf(msg->facility())) {
    processMessage(msg,from_addr);
  }
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


QString Processor::idString() const
{
  return QString::asprintf("Receiver%d:%d",
			   1+receiverNumber(),1+processorNumber());
}


void Processor::rotateLogs(const QDateTime &now)
{
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


void Processor::rotateLogFile(const QString &filename,
			      const QDateTime &now) const
{
  QFileInfo info(filename);
  
  if(((d_log_rotation_size>0)&&
      (info.size()>=d_log_rotation_size))||
     ((d_log_rotation_age>0)&&
      (info.fileTime(QFileDevice::FileBirthTime)<
       now.addDays(-d_log_rotation_age)))) {
    QString new_filename=
      QString::asprintf("%s-%s",filename.toUtf8().constData(),
			now.toString("yyyy-MM-dd").toUtf8().constData());
    if(QFile::exists(new_filename)) {
      int count=0;
      new_filename=
	QString::asprintf("%s-%s",filename.toUtf8().constData(),
			  now.toString("yyyy-MM-dd").toUtf8().constData())+
	QString::asprintf("-%d",1+count);
      while(QFile::exists(new_filename)) {
	count++;
	new_filename=
	  QString::asprintf("%s-%s",filename.toUtf8().constData(),
			    now.toString("yyyy-MM-dd").toUtf8().constData())+
	  QString::asprintf("-%d",1+count);
      }
    }
    if(rename(filename.toUtf8(),new_filename.toUtf8())==0) {
      LocalSyslog(Message::SeverityDebug,
		  "%s rotated log file \"%s\" to \"%s\"",
		  idString().toUtf8().constData(),
		  filename.toUtf8().constData(),
		  new_filename.toUtf8().constData());
    }
    else {
      LocalSyslog(Message::SeverityWarning,"log rotation of \"%s\" failed [%s]",
		  filename.toUtf8().constData(),strerror(errno));
    }
  }
}


bool Processor::expireLogFile(const QString &pathname,
			      const QDateTime &now) const
{
  QFileInfo info(pathname);
  bool ret=false;
  
  if((d_old_log_purge_age>0)&&
     (info.fileTime(QFileDevice::FileBirthTime)<
      now.addDays(-d_old_log_purge_age))) {
    if(unlink(info.filePath().toUtf8())==0) {
      LocalSyslog(Message::SeverityDebug,
		  "%s purged log file \"%s\"",
		  idString().toUtf8().constData(),
		  info.filePath().toUtf8().constData());
      ret=true;
    }
    else {
      LocalSyslog(Message::SeverityWarning,
		  "%s failed to purge log file \"%s\" [%s]",
		  idString().toUtf8().constData(),
		  info.filePath().toUtf8().constData(),
		  strerror(errno));
    }
  }

  return ret;
}


bool Processor::processIf(Message::Facility facility) const
{
  return (MakeMask(((uint32_t)facility))&d_facility_mask)!=0;
}


bool Processor::processIf(Message::Severity severity) const
{
  return (MakeMask(((uint32_t)severity))&d_severity_mask)!=0;
}


QDir *Processor::logRootDirectory() const
{
  return d_log_root_directory;
}


void Processor::logRotationData()
{
  rotateLogs(QDateTime::currentDateTime());
  StartLogRotationTimer();
}


void Processor::StartLogRotationTimer()
{
  QDateTime now=QDateTime::currentDateTime();
  int64_t msec=
    now.msecsTo(QDateTime(now.date(),d_log_rotation_time.addSecs(1)));
  if(msec<0) {
    msec=now.msecsTo(QDateTime(now.date().addDays(1),
			       d_log_rotation_time.addSecs(1)));
  }
  d_log_rotation_timer->start(msec);
  LocalSyslog(Message::SeverityDebug,
	      "Receiver%d:%d: started log rotation timer: %ld ms",
	      1+receiverNumber(),1+processorNumber(),msec);
}


uint32_t Processor::MakeSeverityMask(const QString &params,bool *ok,
				     QString *err_msg) const
{
  uint32_t mask=0;
  
  *ok=false;

  QStringList f0=params.split(",",Qt::KeepEmptyParts);
  for(int i=0;i<f0.size();i++) {
    bool loop_ok=false;
    QString str=f0.at(i).trimmed();
    bool plus=false;
    if(str.right(1)=="+") {
      plus=true;
      str=str.left(str.length()-1).trimmed();
    }
    unsigned num=str.toUInt(&loop_ok);
    if(loop_ok&&(num<Message::SeverityLast)) {
      mask=mask|MakeMask(num);
      if(plus) {
	for(unsigned j=0;j<num;j++) {
	  mask=mask|(MakeMask(j));
	}
      }
    }
    else {
      Message::Severity severity=Message::severityFromString(str);
      if(severity!=Message::SeverityLast) {
	mask=mask|MakeMask((uint32_t)severity);
	if(plus) {
	  for(unsigned j=0;j<(unsigned)severity;j++) {
	    mask=mask|(MakeMask(j));
	  }
	}
	continue;
      }
      else {
	*ok=false;
	*err_msg=QObject::tr("unrecognized severity")+" \""+str+"\"";
	break;
      }
    }
  }

  return mask;
}


uint32_t Processor::MakeFacilityMask(const QString &params,bool *ok,
				     QString *err_msg) const
{
  uint32_t mask=0;
  
  *ok=false;

  QStringList f0=params.split(",",Qt::KeepEmptyParts);
  for(int i=0;i<f0.size();i++) {
    bool loop_ok=false;
    QString str=f0.at(i).trimmed();
    unsigned num=str.toUInt(&loop_ok);
    if(loop_ok&&(num<Message::FacilityLast)) {
      mask=mask|(1<<num);
    }
    else {
      Message::Facility facility=Message::facilityFromString(str);
      if(facility!=Message::FacilityLast) {
	mask=mask|MakeMask((uint32_t)facility);
	continue;
      }
      else {
	*ok=false;
	*err_msg=QObject::tr("unrecognized facility")+" \""+str+"\"";
	break;
      }
    }
  }

  return mask;
}


uint32_t Processor::MakeMask(uint32_t num) const
{
  return 1<<(uint32_t)num;
}
