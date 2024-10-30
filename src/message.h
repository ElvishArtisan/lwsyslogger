// message.h
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

#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdint.h>

#include <QByteArray>
#include <QDateTime>
#include <QObject>
#include <QString>

#define SYSLOG_VERSION 1
#define UTF8_BOM (QByteArray(1,0xEF)+QByteArray(1,0xBB)+QByteArray(1,0xBF))

class Message : public QObject
{
 public:
  enum Facility {FacilityKern=0,FacilityUser=1,FacilityMail=2,FacilityDaemon=3,
    FacilityAuth=4,FacilitySyslog=5,FacilityLpr=6,FacilityNews=7,
    FacilityUucp=8,FacilityCron=9,FacilityAuthpriv=10,FacilityFtp=11,
    FacilityNtp=12,FacilityAudit=13,FacilityAlert=14,FacilityClock=15,
    FacilityLocal0=16,FacilityLocal1=17,FacilityLocal2=18,FacilityLocal3=19,
    FacilityLocal4=20,FacilityLocal5=21,FacilityLocal6=22,FacilityLocal7=23,
    FacilityLast=24};
  enum Severity {SeverityEmerg=0,SeverityAlert=1,SeverityCrit=2,SeverityErr=3,
    SeverityWarning=4,SeverityNotice=5,SeverityInfo=6,SeverityDebug=7,
    SeverityLast=8};
  Message(const QByteArray &data);
  Message(Message::Severity severity,const QString &msg);
  Message();
  bool isValid() const;
  int version() const;
  unsigned priority() const;
  Facility facility() const;
  Severity severity() const;
  QDateTime timestamp() const;
  void setTimestamp(const QDateTime &dt);
  QString hostName() const;
  QString appName() const;
  QString procId() const;
  QString msgId() const;
  QString msg() const;
  QByteArray toByteArray();
  QString resolveWildcards(const QString &fmt);
  void clear();
  QString dump() const;
  static QString facilityString(Facility facility);
  static Facility facilityFromString(const QString &str);
  static QString severityString(Severity severity);
  static Severity severityFromString(const QString &str);

 protected:
  QString nillified(const QString &str);
  
 private:
  void ParseRfc5424(const QByteArray &data);
  void ParseRfc3164(const QByteArray &data);
  bool d_valid;
  int d_version;
  Facility d_facility;
  Severity d_severity;
  QDateTime d_timestamp;
  QString d_host_name;
  QString d_app_name;
  QString d_proc_id;
  QString d_msg_id;
  QString d_msg;
};


#endif  // MESSAGE_H
