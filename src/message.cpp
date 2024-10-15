// message.cpp
//
// Abstract a Syslog Message
//
// For basic concepts regarding  Syslog Messages, see RFC 5424 Section 6)
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

#include <syslog.h>

#include <QStringList>

#include "message.h"

Message::Message(const QByteArray &data)
{
  bool ok=false;
  
  clear();
  
  //
  // Read PRI
  //
  if((data.indexOf('<')==0)&&(data.indexOf('>')<=4)) {
    unsigned prio=QString(data.mid(1,data.indexOf('>')-1)).toUInt(&ok);
    if((!ok)||(prio>191)) {
      syslog(LOG_WARNING,"invalid PRIO received");
      return;
    }
    d_severity=0x07&prio;
    d_facility=prio>>3;
    //
    // Determine Message Protocol
    //
    if(data.at(data.indexOf('>')+1)=='1') {  // RFC-5424 v1 Format
      ParseRfc5424(data.right(data.size()-data.indexOf('>')-2));
    }
    else {   // RFC-3164 Format
      ParseRfc3164(data.right(data.size()-data.indexOf('>')-1));
    }
  }
}


Message::Message()
{
  clear();
}


bool Message::isValid() const
{
  return d_valid;
}


int Message::version() const
{
  return d_version;
}


unsigned Message::priority() const
{
  unsigned prio=0;

  if(d_facility<=23) {
    prio=8*d_facility;
  }
  if(d_severity<=7) {
    prio+=d_severity;
  }
  
  return prio;
}


unsigned Message::facility() const
{
  return d_facility;
}


unsigned Message::severity() const
{
  return d_severity;
}


QDateTime Message::timestamp() const
{
  return d_timestamp;
}


QString Message::hostName() const
{
  return d_host_name;
}


QString Message::appName() const
{
  return d_app_name;
}


QString Message::procId() const
{
  return d_proc_id;
}


QString Message::msgId() const
{
  return d_msg_id;
}


QString Message::msg() const
{
  return d_msg;
}


QByteArray Message::toByteArray()
{
  d_timestamp=QDateTime::currentDateTime();
  QByteArray ret;

  ret+=QString::asprintf("<%u>",priority()).toUtf8();
  ret+=QString::asprintf("%u ",SYSLOG_VERSION).toUtf8();
  ret+=(d_timestamp.toString("yyyy-MM-ddThh:mm:ss.zzz")+" ").toUtf8();
  ret+=(nillified(d_host_name)+" ").toUtf8();
  ret+=(nillified(d_app_name)+" ").toUtf8();
  ret+=(nillified(d_proc_id)+" ").toUtf8();
  ret+=QString("- ").toUtf8();  // No structured data
  ret+=(nillified(d_msg_id)+" ").toUtf8();
  ret+=UTF8_BOM;
  ret+=d_msg.toUtf8();

  return ret;
}


void Message::clear()
{
  d_valid=false;
  d_facility=0;
  d_severity=0;
  d_timestamp=QDateTime();
  d_host_name=QString();
  d_app_name=QString();
  d_proc_id=QString();
  d_msg_id=QString();
  d_msg=QString();
}


QString Message::dump() const
{
  QString ret;
  
  if(d_version==0) {
    ret+="Version: 0\n";
    ret+="Timestamp: XXXX-"+d_timestamp.toString("MM-dd hh:mm:ss")+"\n";
    ret+=QString::asprintf("Facility: %s\n",
			   Message::facilityString(d_facility).
			   toUtf8().constData());
    ret+=QString::asprintf("Severity: %s\n",
			   Message::severityString(d_severity).
			   toUtf8().constData());
    ret+="Hostname: "+d_host_name+"\n";
    ret+="Msg: "+d_msg+"\n";
  }

  return ret;
}


QString Message::facilityString(unsigned facility)
{
  QString ret=QObject::tr("Unknown")+QString::asprintf(" [%u]",facility);

  switch(facility) {
  case LOG_KERN:
    ret="LOG_KERN [0]";
    break;

  case LOG_USER:
    ret="LOG_USER [1]";
    break;

  case LOG_MAIL:
    ret="LOG_MAIL [2]";
    break;

  case LOG_DAEMON:
    ret="LOG_DAEMON [3]";
    break;

  case LOG_AUTH:
    ret="LOG_AUTH [4]";
    break;

  case LOG_SYSLOG:
    ret="LOG_SYSLOG [5]";
    break;

  case LOG_LPR:
    ret="LOG_LPR [6]";
    break;

  case LOG_NEWS:
    ret="LOG_NEWS [7]";
    break;

  case LOG_UUCP:
    ret="LOG_UUCP [8]";
    break;

  case LOG_CRON:
    ret="LOG_CRON [9]";
    break;

  case LOG_AUTHPRIV:
    ret="LOG_AUTHPRIV [10]";
    break;

  case LOG_FTP:
    ret="LOG_FTP [11]";
    break;

  case 12:
    ret="NTP Subsystem [12]";
    break;

  case 13:
    ret="Log Audit [13]";
    break;

  case 14:
    ret="Log Alert [14]";
    break;

  case 15:
    ret="Clock Daemon [15]";
    break;

  case LOG_LOCAL0:
    ret="LOG_LOCAL0 [16]";
    break;

  case LOG_LOCAL1:
    ret="LOG_LOCAL1 [17]";
    break;

  case LOG_LOCAL2:
    ret="LOG_LOCAL2 [18]";
    break;

  case LOG_LOCAL3:
    ret="LOG_LOCAL3 [19]";
    break;

  case LOG_LOCAL4:
    ret="LOG_LOCAL4 [20]";
    break;

  case LOG_LOCAL5:
    ret="LOG_LOCAL5 [21]";
    break;

  case LOG_LOCAL6:
    ret="LOG_LOCAL6 [22]";
    break;

  case LOG_LOCAL7:
    ret="LOG_LOCAL7 [23]";
    break;
  }
  return ret;
}


QString Message::severityString(unsigned severity)
{
  QString ret=QObject::tr("Unknown")+QString::asprintf("%u",severity);

  switch(severity) {
  case LOG_EMERG:
    ret="LOG_EMERG [0]";
    break;

  case LOG_ALERT:
    ret="LOG_ALERT [1]";
    break;

  case LOG_CRIT:
    ret="LOG_CRIT [2]";
    break;

  case LOG_ERR:
    ret="LOG_ERR [3]";
    break;

  case LOG_WARNING:
    ret="LOG_WARNING [4]";
    break;

  case LOG_NOTICE:
    ret="LOG_NOTICE [5]";
    break;

  case LOG_INFO:
    ret="LOG_INFO [6]";
    break;

  case LOG_DEBUG:
    ret="LOG_DEBUG [7]";
    break;
  }

  return ret;
}

QString Message::nillified(const QString &str)
{
  if(str.trimmed().isEmpty()) {
    return "-";
  }
  return str.trimmed();
}


void Message::ParseRfc5424(const QByteArray &data)
{
}


void Message::ParseRfc3164(const QByteArray &data)
{
  //
  // Traditional BSD-Style Format
  //
  d_timestamp=QDateTime::fromString(QString(data.mid(0,15)),"MMM dd hh:mm:ss");
  if(d_timestamp.isValid()) {
    QStringList f0=QString::fromUtf8(data.right(data.size()-16)).split(" ",Qt::KeepEmptyParts);
    d_host_name=f0.first();
    f0.removeFirst();
    d_msg=f0.join(" ");
    d_version=0;
    d_valid=true;
    //    printf("HOSTNAME: |%s|\n",d_host_name.toUtf8().constData());
    //    printf("MSG: |%s|\n",d_msg.toUtf8().constData());
  }
}
