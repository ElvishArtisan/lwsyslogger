// addressfilter.cpp
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

#include "addressfilter.h"

AddressFilter::AddressFilter()
{
}


void AddressFilter::addSubnet(const QHostAddress &addr,int netmask)
{
  QPair<QHostAddress,int> subnet(addr,netmask);
  if(!d_subnets.contains(subnet)) {
    d_subnets.push_back(subnet);
  }
}


bool AddressFilter::contains(const QHostAddress &addr) const
{
  for(int i=0;i<d_subnets.size();i++) {
    /*
    printf("checking %s against %s/%d\n",
	   addr.toString().toUtf8().constData(),
	   d_subnets.at(i).first.toString().toUtf8().constData(),
	   d_subnets.at(i).second);
    */
    if(addr.isInSubnet(d_subnets.at(i))) {
      // printf("MATCH\n");
      return true;
    }
  }
  //  printf("NO MATCH\n");
  return false;
}
