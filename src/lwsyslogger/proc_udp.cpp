// proc_udp.cpp
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

#include "proc_udp.h"

ProcUdp::ProcUdp(const QString &id,Profile *p,QObject *parent)
  : Processor(id,p,parent)
{
  bool ok=false;
  
  QStringList strings=p->stringValues("Processor",id,"DestinationAddress");
  if(strings.isEmpty()) {
    fprintf(stderr,"lwsyslogger: missing parameter \"DestinationAddress\"\n");
    exit(1);
  }
  for(int i=0;i<strings.size();i++) {
    QStringList f0=strings.at(i).split(":",Qt::KeepEmptyParts);
    if(f0.size()!=2) {
      fprintf(stderr,
       "lwsyslogger: parameter \"DestinationAddress=%s\" is malformatted.\n",
	      strings.at(i).toUtf8().constData());
      exit(1);
    }
    QHostAddress addr(f0.at(0));
    if(addr.isNull()) {
      fprintf(stderr,
       "lwsyslogger: parameter \"DestinationAddress=%s\" is malformatted.\n",
	      strings.at(i).toUtf8().constData());
      exit(1);
    }
    unsigned port=f0.at(1).toUInt(&ok);
    if((!ok)||(port<1)||(port>0xFFFF)) {
      fprintf(stderr,
       "lwsyslogger: parameter \"DestinationAddress=%s\" is malformatted.\n",
	      strings.at(i).toUtf8().constData());
      exit(1);
    }
    d_destination_addresses.push_back(addr);
    d_destination_ports.push_back(port);
  }
  
  d_send_socket=new QUdpSocket(this);
}


Processor::Type ProcUdp::type() const
{
  return Processor::TypeUdp;
}


void ProcUdp::processMessage(Message *msg,const QHostAddress &from_addr)
{
  for(int i=0;i<d_destination_addresses.size();i++) {
    d_send_socket->writeDatagram(msg->toByteArray(msg->version()),
				 d_destination_addresses.at(i),
				 d_destination_ports.at(i));
  }
}
