// receiver.h
//
// Abstract base class for transport layer receivers.
//
// For basic concepts regarding  Syslog Transport Layer Protocols
// see RFC 5424 Section 5)
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

#ifndef RECEIVER_H
#define RECEIVER_H

#include <QMap>
#include <QObject>

#include "message.h"
#include "profile.h"

class Receiver : public QObject
{
  Q_OBJECT
 public:
  enum Type {TypeUdp=0,TypeLast=1};
  Receiver(const QString &id,Profile *p,QObject *parent=0);
  QString id() const;
  virtual Type type() const=0;
  virtual bool start(QString *err_msg)=0;
  static QString typeString(Type type);
  static Type typeFromString(const QString &str);

 signals:
  void messageReceived(Message *msg,const QHostAddress &from_addr);

 protected:
  Profile *profile() const;
  void lsyslog(Message::Severity severity,const char *fmt,...) const;

 private:
  Profile *d_profile;
  QString d_id;
};


#endif  // RECEIVER_H
