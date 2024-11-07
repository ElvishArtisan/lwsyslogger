// proc_udp.h
//
// Processor for forwarding syslog messagea via UDP.
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

#ifndef PROC_UDP_H
#define PROC_UDP_H

#include <stdint.h>

#include <QUdpSocket>

#include "processor.h"

class ProcUdp : public Processor
{
  Q_OBJECT
 public:
  ProcUdp(const QString &id,Profile *c,QObject *parent=0);
  Processor::Type type() const;

 protected:
  void processMessage(Message *msg,const QHostAddress &from_addr);

 private:
  QList<QHostAddress> d_destination_addresses;
  QList<uint16_t> d_destination_ports;
  QUdpSocket *d_send_socket;
};


#endif  // PROCESSOR_H
