// recv_udp.cpp
//
// UDP Syslog transport protocol
//
// For basic concepts regarding the UDP Syslog Transport Layer Protocol
// see RFC 5426)
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

#include <QNetworkDatagram>

#include "recv_udp.h"

RecvUdp::RecvUdp(const QString &id,Profile *p,QObject *parent)
  : Receiver(id,p,parent)
{
  d_socket=new QUdpSocket(this);
  connect(d_socket,SIGNAL(readyRead()),this,SLOT(readyReadData()));
  connect(d_socket,SIGNAL(error(QAbstractSocket::SocketError)),
	  this,SLOT(errorData(QAbstractSocket::SocketError)));
  
}


Receiver::Type RecvUdp::type() const
{
  return Receiver::TypeUdp;
}


bool RecvUdp::start(QString *err_msg)
{
  QList<int> ivalues=profile()->intValues("Receiver",id(),"Port");
  unsigned udp_port=514;  // Default value
  if(!ivalues.isEmpty()) {
    udp_port=ivalues.last();
  }
  if(udp_port>0xFFFF) {
    *err_msg=QObject::tr("invalid port specified");
    return false;
  }
  if(!d_socket->bind(udp_port)) {
    *err_msg=QObject::tr("failed to bind udp port")+
      QString::asprintf(" %u",udp_port);
    return false;
  }

  return true;
}


void RecvUdp::readyReadData()
{
  QNetworkDatagram dg=d_socket->receiveDatagram();
  Message *msg=new Message(dg.data());
  if(msg->isValid()) {
    emit messageReceived(msg,dg.senderAddress());
  }
  delete msg;
}


void RecvUdp::errorData(QAbstractSocket::SocketError err)
{
}
