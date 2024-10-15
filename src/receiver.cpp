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

#include "receiver.h"

Receiver::Receiver(SyProfile *c,int recv_num,QObject *parent)
  : QObject(parent)
{
  d_config=c;
  d_receiver_number=recv_num;
}


SyProfile *Receiver::config() const
{
  return d_config;
}


int Receiver::receiverNumber() const
{
  return d_receiver_number;
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
