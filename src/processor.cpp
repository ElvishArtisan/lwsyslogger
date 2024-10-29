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

Processor::Processor(const QString &id,Profile *p,QObject *parent)
  : QObject(parent)
{
  bool ok=false;
  QString err_msg;
  QStringList values;
  
  d_id=id;
  d_profile=p;
  d_dry_run=false;
  d_address_filter=new AddressFilter();
  
  values=p->stringValues("Global","Default","LogRoot");
  if(values.isEmpty()) {
    fprintf(stderr,"lwsyslogger: unable to load location of \"LogRoot\"\n");
    exit(1);
  }
  d_log_root_directory=new QDir(values.last());

  //
  // Facility Values
  //
  QStringList strings=d_profile->stringValues("Processor",id,"Facility");
  d_facility_mask=MakeFacilityMask(strings.join(","),&ok,&err_msg);
  if(!ok) {
    fprintf(stderr,
	  "lwsyslogger: invalid facility string \"%s\" for processor \"%s\"\n",
	    strings.join(",").toUtf8().constData(),id.toUtf8().constData());
    exit(1);
  }
  QString maskstr;
  for(int i=0;i<25;i++) {
    if((d_facility_mask&(1<<i))!=0) {
      maskstr+=Message::facilityString((Message::Facility)i)+",";
    }
  }
  maskstr=maskstr.left(maskstr.length()-1);
  lsyslog(Message::SeverityDebug,"processing facilities: %s",
  	  maskstr.toUtf8().constData());

  //
  // Severity Values
  //
  strings=d_profile->stringValues("Processor",id,"Severity");
  d_severity_mask=MakeSeverityMask(strings.join(","),&ok,&err_msg);
  if(!ok) {
    fprintf(stderr,
	  "lwsyslogger: invalid severity string \"%s\" for processor \"%s\"\n",
	    strings.join(",").toUtf8().constData(),id.toUtf8().constData());
    exit(1);
  }
  maskstr="";
  for(int i=0;i<9;i++) {
    if((d_severity_mask&(1<<i))!=0) {
      maskstr+=Message::severityString((Message::Severity)i)+",";
    }
  }
  maskstr=maskstr.left(maskstr.length()-1);
  lsyslog(Message::SeverityDebug,"processing severities: %s",
	  maskstr.toUtf8().constData());
  
  //
  // Upstream Addresses
  //
  strings=d_profile->stringValues("Processor",id,"UpstreamAddress");
  for(int i=0;i<strings.size();i++) {
    QStringList f0=strings.at(i).split(",",Qt::SkipEmptyParts);
    for(int j=0;j<f0.size();j++) {
      QStringList f1=f0.at(j).split("/",Qt::KeepEmptyParts);
      if(f1.size()==2) {
	QHostAddress addr(f1.at(0));
	if(addr.isNull()) {
	  fprintf(stderr,
	      "lwsyslogger: invalid CIDR subnet \"%s\" for processor \"%s\"\n",
		  f0.at(j).toUtf8().constData(),d_id.toUtf8().constData());
	  exit(1);
	}
	int netmask=f1.at(1).toInt(&ok);
	if((!ok)||(netmask<0)) {
	  fprintf(stderr,
	      "lwsyslogger: invalid CIDR subnet \"%s\" for processor \"%s\"\n",
		  f0.at(j).toUtf8().constData(),d_id.toUtf8().constData());
	  exit(1);
	}
	d_address_filter->addSubnet(addr,netmask);
      }
      else {
	fprintf(stderr,
	      "lwsyslogger: invalid CIDR subnet \"%s\" for processor \"%s\"\n",
		f0.at(j).toUtf8().constData(),d_id.toUtf8().constData());
	exit(1);
      }
    }
  }
  lsyslog(Message::SeverityDebug,"processing addresses: %s",
	  d_address_filter->subnets().toUtf8().constData());

  //
  // Log Rotation Values
  //
  d_log_rotation_age=0;  // Default value
  QList<int> ivalues=p->intValues("Processor",id,"LogRotationAge");
  if(!ivalues.isEmpty()) {
    d_log_rotation_age=ivalues.last();
  }
  d_log_rotation_size=0;  // Default value
  ivalues=p->intValues("Processor",id,"LogRotationSize");
  if(!ivalues.isEmpty()) {
    d_log_rotation_size=ivalues.last();
  }
  d_old_log_purge_age=0;  // Default value
  ivalues=p->intValues("Processor",id,"OldLogPurgeAge");
  if(!ivalues.isEmpty()) {
    d_old_log_purge_age=ivalues.last();
  }
  d_log_rotation_time=QTime();  // Default value
  QList<QTime> tvalues=p->timeValues("Processor",id,"LogRotationTime");
  if(!tvalues.isEmpty()) {
    d_log_rotation_time=tvalues.last();
  }
  if(!d_log_rotation_time.isValid()) {
    fprintf(stderr,"invalid time value in LogRotationTime\n");
    exit(1);
  }
  d_log_rotation_timer=new QTimer(this);
  d_log_rotation_timer->setSingleShot(true);
  connect(d_log_rotation_timer,SIGNAL(timeout()),this,SLOT(logRotationData()));
  StartLogRotationTimer();
}


QString Processor::id() const
{
  return d_id;
}


bool Processor::dryRun() const
{
  return d_dry_run;
}


void Processor::setDryRun(bool state)
{
  d_dry_run=state;
}


bool Processor::start(QString *err_msg)
{
  return true;
}


Profile *Processor::config() const
{
  return d_profile;
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


void Processor::process(Message *msg,const QHostAddress &from_addr)
{
  if(((MakeMask(((uint32_t)msg->facility()))&d_facility_mask)!=0)&&
     ((MakeMask(((uint32_t)msg->severity()))&d_severity_mask)!=0)&&
     d_address_filter->contains(from_addr)) {
    processMessage(msg,from_addr);
  }
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
    if(d_dry_run) {
      printf("Processor %s: would rotate file \"%s\" to \"%s\"\n",
	     id().toUtf8().constData(),
	     filename.toUtf8().constData(),
	     new_filename.toUtf8().constData());
    }
    else {
      if(rename(filename.toUtf8(),new_filename.toUtf8())==0) {
	lsyslog(Message::SeverityDebug,
		    "rotated log file \"%s\" to \"%s\"",
		    filename.toUtf8().constData(),
		    new_filename.toUtf8().constData());
      }
      else {
	lsyslog(Message::SeverityWarning,"log rotation of \"%s\" failed [%s]",
		    filename.toUtf8().constData(),strerror(errno));
      }
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
    if(d_dry_run) {
      printf("Processor %s: would purge file \"%s\"\n",
	     id().toUtf8().constData(),
	     info.filePath().toUtf8().constData());
    }
    else {
      if(unlink(info.filePath().toUtf8())==0) {
	lsyslog(Message::SeverityDebug,
		"purged log file \"%s\"",
		info.filePath().toUtf8().constData());
	ret=true;
      }
      else {
	lsyslog(Message::SeverityWarning,
		"failed to purge log file \"%s\" [%s]",
		info.filePath().toUtf8().constData(),strerror(errno));
      }
    }
  }

  return ret;
}


QDir *Processor::logRootDirectory() const
{
  return d_log_root_directory;
}


void Processor::lsyslog(Message::Severity severity,const char *fmt,...) const
{
  char buffer[1024];

  va_list args;
  va_start(args,fmt);
  vsnprintf(buffer,1024,fmt,args);
  va_end(args);
  LocalSyslog(severity,"processor %s: %s",d_id.toUtf8().constData(),buffer);
}


void Processor::logRotationData()
{
  rotateLogs(QDateTime::currentDateTime());
  StartLogRotationTimer();
}


void Processor::StartLogRotationTimer()
{
  if(!d_log_rotation_time.isNull()) {
    QDateTime now=QDateTime::currentDateTime();
    int64_t msec=
      now.msecsTo(QDateTime(now.date(),d_log_rotation_time.addSecs(1)));
    if(msec<0) {
      msec=now.msecsTo(QDateTime(now.date().addDays(1),
				 d_log_rotation_time.addSecs(1)));
    }
    d_log_rotation_timer->start(msec);
    lsyslog(Message::SeverityDebug,"started log rotation timer: %ld ms",msec);
  }
}


uint32_t Processor::MakeSeverityMask(const QString &params,bool *ok,
				     QString *err_msg) const
{
  uint32_t mask=0;
  
  *ok=true;

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
  
  *ok=true;

  QStringList f0=params.split(",",Qt::KeepEmptyParts);
  for(int i=0;i<f0.size();i++) {
    bool loop_ok=false;
    bool negavate=false;
    QString str=f0.at(i).trimmed();
    if(str.left(1)=="-") {
      negavate=true;
      str=str.right(str.length()-1);
    }
    unsigned num=str.toUInt(&loop_ok);
    if(loop_ok&&(num<Message::FacilityLast)) {
      if(negavate) {
	mask=mask&(0xFFFFFFFF^MakeMask(num));
      }
      else {
	mask=mask|MakeMask(num);
      }
    }
    else {
      Message::Facility facility=Message::facilityFromString(str);
      if(facility!=Message::FacilityLast) {
	if(negavate) {
	  mask=mask&(0xFFFFFFFF^MakeMask(facility));
	}
	else {
	  mask=mask|MakeMask((uint32_t)facility);
	}
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
