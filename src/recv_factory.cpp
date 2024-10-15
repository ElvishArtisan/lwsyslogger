// recv_factory.cpp
//
// Receiver Factory
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

#include "recv_udp.h"

Receiver *ReceiverFactory(Receiver::Type type,SyProfile *c,int recv_num,
			  QObject *parent)
{
  Receiver *recv=NULL;
  
  switch(type) {
  case Receiver::TypeUdp:
    recv=new RecvUdp(c,recv_num,parent);
    break;

  case Receiver::TypeLast:
    break;
  }

  return recv;
}
