// addressfilter.h
//
// Check IP addresses against a whitelist
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

#ifndef ADDRESSFILTER_H
#define ADDRESSFILTER_H

#include <QHostAddress>
#include <QList>
#include <QPair>

class AddressFilter
{
 public:
  AddressFilter();
  void addSubnet(const QHostAddress &addr,int netmask);
  bool contains(const QHostAddress &addr) const;
  QString subnets() const;
  
 private:
  QList<QPair<QHostAddress,int> > d_subnets;
};


#endif  // ADDRESSFILTER_H
