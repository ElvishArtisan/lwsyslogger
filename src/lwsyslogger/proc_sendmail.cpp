// proc_sendmail.cpp
//
// Processor for sending mail.
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

#include "proc_sendmail.h"
#include "sendmail.h"

ProcSendmail::ProcSendmail(const QString &id,Profile *p,QObject *parent)
  : Processor(id,p,parent)
{
  QStringList strings=p->stringValues("Processor",id,"EmailFromAddress");
  if(strings.isEmpty()) {
    fprintf(stderr,"lwsyslogger: missing FromAddress in processor %s\n",
	    id.toUtf8().constData());
    exit(1);
  }
  d_from_address=strings.last();

  d_to_addresses=p->stringValues("Processor",id,"EmailToAddress");
  if(d_to_addresses.isEmpty()) {
    fprintf(stderr,"lwsyslogger: missing ToAddress in processor %s\n",
	    id.toUtf8().constData());
    exit(1);
  }

  d_subject_line=QObject::tr("Syslog Alert");
  strings=p->stringValues("Processor",id,"EmailSubjectLine");
  if(!strings.isEmpty()) {
    d_subject_line=strings.last();  
  }

  //
  // Throttling Parameters
  //
  d_throttle_limit=0;
  QList<int> ints=p->intValues("Processor",id,"EmailThrottleLimit");
  if(!ints.isEmpty()) {
    d_throttle_limit=ints.last();
  }

  d_throttle_period=0;
  ints=p->intValues("Processor",id,"EmailThrottlePeriod");
  if(!ints.isEmpty()) {
    d_throttle_period=ints.last();
  }

  d_throttle_timer=new QTimer(this);
  d_throttle_timer->setSingleShot(true);
  connect(d_throttle_timer,SIGNAL(timeout()),this,SLOT(throttleTimeoutData()));
}


Processor::Type ProcSendmail::type() const
{
  return Processor::TypeSendmail;
}


bool ProcSendmail::start(QString *err_msg)
{
  if((d_throttle_limit>0)&&(d_throttle_period>0)) {
    d_throttle_counter=0;
    d_throttle_timer->start(1000*d_throttle_period);
    lsyslog(Message::SeverityDebug,
	    "limiting mail messages to no more than %d per %d seconds",
	    d_throttle_limit,d_throttle_period);
  }
  return true;
}


void ProcSendmail::processMessage(Message *msg,const QHostAddress &from_addr)
{
  QString err_msg;
  
  if(d_throttle_timer->isActive()) {
    d_throttle_counter++;
    if(d_throttle_counter==(1+d_throttle_limit)) {
      lsyslog(Message::SeverityWarning,"throttling message warnings");
      QString warning=QString::asprintf("%s: throttling message warnings",
					id().toUtf8().constData());
      if(!SendMail(&err_msg,warning,warning,d_from_address,d_to_addresses)) {
	lsyslog(Message::SeverityWarning,"sendmail failed [%s]",
		err_msg.toUtf8().constData());
      }
    }
    if(d_throttle_counter>d_throttle_limit) {
      return;
    }
  }
  QString subj=msg->resolveWildcards(d_subject_line);
  QString body=msg->resolveWildcards(messageTemplate());
  if(!SendMail(&err_msg,subj,body,d_from_address,d_to_addresses)) {
    lsyslog(Message::SeverityWarning,"sendmail failed [%s]",
	    err_msg.toUtf8().constData());
  }
}


void ProcSendmail::throttleTimeoutData()
{
  d_throttle_counter=0;
  d_throttle_timer->start(1000*d_throttle_period);
}
