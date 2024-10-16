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

RecvUdp::RecvUdp(SyProfile *c,int recv_num,QObject *parent)
  : Receiver(c,recv_num,parent)
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
  bool ok=false;

  unsigned udp_port=config()->
    intValue(QString::asprintf("Receiver%d",1+receiverNumber()),"Port",514,&ok);
  if((!ok)||(udp_port>0xFFFF)) {
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
    processMessage(msg,dg.senderAddress());
  }
  delete msg;
}


void RecvUdp::errorData(QAbstractSocket::SocketError err)
{
}
