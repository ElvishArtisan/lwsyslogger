// receiver.cpp
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

#include "local_syslog.h"
#include "proc_factory.h"
#include "receiver.h"

Receiver::Receiver(const QString &id,Profile *p,QObject *parent)
  : QObject(parent)
{
  d_id=id;
  d_profile=p;
}


QString Receiver::id() const
{
  return d_id;
}


Profile *Receiver::profile() const
{
  return d_profile;
}


void Receiver::lsyslog(Message::Severity severity,const char *fmt,...) const
{
  char buffer[1024];

  va_list args;
  va_start(args,fmt);
  vsnprintf(buffer,1024,fmt,args);
  va_end(args);
  LocalSyslog(severity,"receiver %s: %s",d_id.toUtf8().constData(),buffer);
}


QString Receiver::typeString(Receiver::Type type)
{
  QString ret="UNKNOWN";
  
  switch(type) {
  case Receiver::TypeUdp:
    ret="UDP";
    break;

  case Receiver::TypeLast:
    break;
  }

  return ret;
}


Receiver::Type Receiver::typeFromString(const QString &str)
{
  for(int i=0;i<Receiver::TypeLast;i++) {
    if(Receiver::typeString((Receiver::Type)i).toLower()==str.toLower()) {
      return (Receiver::Type)i;
    }
  }
  
  return Receiver::TypeLast;
}


void Receiver::forwardMessage(Message *msg,const QHostAddress &from_addr)
{
  bool ok=false;
  uint32_t v4_addr=from_addr.toIPv4Address(&ok);
  if(ok) {
    emit messageReceived(msg,QHostAddress(v4_addr));
  }
  else {
    emit messageReceived(msg,from_addr);
  }
}
