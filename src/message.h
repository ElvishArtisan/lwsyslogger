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

#include <QByteArray>
#include <QDateTime>
#include <QObject>
#include <QString>

#define SYSLOG_VERSION 1
#define UTF8_BOM (QByteArray(1,0xEF)+QByteArray(1,0xBB)+QByteArray(1,0xBF))

class Message : public QObject
{
 public:
  Message(const QByteArray &data);
  Message();
  bool isValid() const;
  int version() const;
  unsigned priority() const;
  unsigned facility() const;
  unsigned severity() const;
  QDateTime timestamp() const;
  QString hostName() const;
  QString appName() const;
  QString procId() const;
  QString msgId() const;
  QString msg() const;
  QByteArray toByteArray();
  void clear();
  QString dump() const;
  static QString facilityString(unsigned facility);
  static QString severityString(unsigned severity);
  
 protected:
  QString nillified(const QString &str);
  
 private:
  void ParseRfc5424(const QByteArray &data);
  void ParseRfc3164(const QByteArray &data);
  bool d_valid;
  int d_version;
  unsigned d_facility;
  unsigned d_severity;
  QDateTime d_timestamp;
  QString d_host_name;
  QString d_app_name;
  QString d_proc_id;
  QString d_msg_id;
  QString d_msg;
};


#endif  // MESSAGE_H
