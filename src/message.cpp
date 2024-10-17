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
    d_severity=(Message::Severity)(0x07&prio);
    d_facility=(Message::Facility)(prio>>3);
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


Message::Facility Message::facility() const
{
  return d_facility;
}


Message::Severity Message::severity() const
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


QString Message::resolveWildcards(const QString &fmt)
{
  QString ret=fmt;

  ret.replace("%f",QString::asprintf("%u",d_facility));   // Facility (numeric)
  ret.replace("%F",Message::facilityString(d_facility));  // Facility (symbolic)
  ret.replace("%h",d_host_name);                          // Hostname
  ret.replace("%m",d_msg);                                // MSG
  ret.replace("%p",QString::asprintf("%u",priority()));   // PRIO
  ret.replace("%P",QString::asprintf("<%u>",priority())); // PRIO (decorated)
  ret.replace("%s",QString::asprintf("%u",d_severity));   // Severity (numeric)
  ret.replace("%S",Message::severityString(d_severity));  // Severity (symbolic)
  ret.replace("%t",d_timestamp.
	      toString("MMM dd hh:mm:ss"));               // Timestamp (BSD)
  ret.replace("%T",d_timestamp.
	      toString("yyyy-MM-ddThh:mm:ss"));         // Timestamp (RFC-5424)
  return ret;
}


void Message::clear()
{
  d_valid=false;
  d_facility=Message::FacilityLast;
  d_severity=Message::SeverityLast;;
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
    ret+="Protocol Version: 0 [Traditional BSD]\n";
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


QString Message::facilityString(Message::Facility facility)
{
  QString ret=QObject::tr("UNKNOWN")+QString::asprintf(" [%u]",facility);

  switch(facility) {
  case Message::FacilityKern:      // 0
    ret="LOG_KERN";
    break;

  case Message::FacilityUser:      // 1
    ret="LOG_USER";
    break;

  case Message::FacilityMail:      // 2
    ret="LOG_MAIL";
    break;

  case Message::FacilityDaemon:    // 3
    ret="LOG_DAEMON";
    break;

  case Message::FacilityAuth:      // 4
    ret="LOG_AUTH";
    break;

  case Message::FacilitySyslog:    // 5
    ret="LOG_SYSLOG";
    break;

  case Message::FacilityLpr:       // 6
    ret="LOG_LPR";
    break;

  case Message::FacilityNews:      // 7
    ret="LOG_NEWS";
    break;

  case Message::FacilityUucp:      // 8
    ret="LOG_UUCP";
    break;

  case Message::FacilityCron:      // 9
    ret="LOG_CRON";
    break;

  case Message::FacilityAuthpriv:  // 10
    ret="LOG_AUTHPRIV";
    break;

  case Message::FacilityFtp:       // 11
    ret="LOG_FTP";
    break;

  case Message::FacilityNtp:        // 12
    ret="LOG_NTP";
    break;

  case Message::FacilityAudit:           // 13
    ret="LOG_AUDIT";
    break;

  case Message::FacilityAlert:           // 14
    ret="LOG_ALERT";
    break;

  case Message::FacilityClock:           // 15
    ret="LOG_CLOCK";
    break;

  case Message::FacilityLocal0:    // 16
    ret="LOG_LOCAL0";
    break;

  case Message::FacilityLocal1:    // 17
    ret="LOG_LOCAL1";
    break;

  case Message::FacilityLocal2:    // 18
    ret="LOG_LOCAL2";
    break;

  case Message::FacilityLocal3:    // 19
    ret="LOG_LOCAL3";
    break;

  case Message::FacilityLocal4:    // 20
    ret="LOG_LOCAL4";
    break;

  case Message::FacilityLocal5:    // 21
    ret="LOG_LOCAL5";
    break;

  case Message::FacilityLocal6:    // 22
    ret="LOG_LOCAL6";
    break;

  case Message::FacilityLocal7:    // 23
    ret="LOG_LOCAL7";
    break;

  case Message::FacilityLast:      // 24
    break;
  }
  return ret;
}


Message::Facility Message::facilityFromString(const QString &str)
{
  for(int i=0;i<Message::FacilityLast;i++) {
    if(Message::facilityString((Message::Facility)i).toUpper()==
       str.trimmed().toUpper()) {
      return (Message::Facility)i;
    }
  }
  
  return Message::FacilityLast;
}


QString Message::severityString(Message::Severity severity)
{
  QString ret=QObject::tr("UNKNOWN")+QString::asprintf("%u",severity);

  switch(severity) {
  case Message::SeverityEmerg:     // 0
    ret="LOG_EMERG";
    break;

  case Message::SeverityAlert:     // 1
    ret="LOG_ALERT";
    break;

  case Message::SeverityCrit:      // 2
    ret="LOG_CRIT";
    break;

  case Message::SeverityErr:       // 3
    ret="LOG_ERR";
    break;

  case Message::SeverityWarning:   // 4
    ret="LOG_WARNING";
    break;

  case Message::SeverityNotice:    // 5
    ret="LOG_NOTICE";
    break;

  case Message::SeverityInfo:      // 6
    ret="LOG_INFO";
    break;

  case Message::SeverityDebug:     // 7
    ret="LOG_DEBUG";
    break;

  case Message::SeverityLast:      // 8
    break;
  }

  return ret;
}


Message::Severity Message::severityFromString(const QString &str)
{
  for(int i=0;i<Message::SeverityLast;i++) {
    if(Message::severityString((Message::Severity)i).toUpper()==
       str.trimmed().toUpper()) {
      return (Message::Severity)i;
    }
  }
  
  return Message::SeverityLast;
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

  //
  // Hack to fudge a year for a BSD-style timestamp
  // WARNING: This races for times near the year rollover!
  //
  QStringList f0=QString(data.mid(0,15)).split(" ",Qt::SkipEmptyParts);
  QString year=QDate::currentDate().toString("yyyy");
  d_timestamp=
    QDateTime::fromString(year+" "+f0.join(" "),"yyyy MMM d hh:mm:ss");

  if(d_timestamp.isValid()) {
    QStringList f1=QString::fromUtf8(data.right(data.size()-16)).
      split(" ",Qt::KeepEmptyParts);
    d_host_name=f1.first().trimmed();
    f1.removeFirst();
    d_msg=f1.join(" ").trimmed();
    d_version=0;
    d_valid=true;
  }
}
