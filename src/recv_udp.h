// recv_udp.h
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

#ifndef RECV_UDP_H
#define RECV_UDP_H

#include <QUdpSocket>

#include "receiver.h"

class RecvUdp : public Receiver
{
  Q_OBJECT
 public:
  RecvUdp(SyProfile *c,int recv_num,QObject *parent);
  Receiver::Type type() const;
  bool start(QString *err_msg);
  
 private slots:
  void readyReadData();
  void errorData(QAbstractSocket::SocketError err);

 private:
  QUdpSocket *d_socket;
};


#endif  // RECV_UDP_H
