// lwlogger.h
//
// Simple syslog logger for Livewire sites
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

#ifndef LWLOGGER_H
#define LWLOGGER_H

#include <QList>
#include <QObject>

#include <sy5/syprofile.h>

#include "receiver.h"

//
// Defaults
//
#define DEFAULT_CONFIG_FILENAME "/etc/lwlogger.conf"

//
// Global RIPCD Definitions
//
#define LWLOGGER_USAGE "[--config=<filename>] [-d]\n\n"

class MainObject : public QObject
{
  Q_OBJECT
 public:
  MainObject(QObject *parent=0);

 private slots:
  void messageReceivedData(Message *msg,const QHostAddress &from_addr);
   
 private:
  QList<Receiver *> d_receivers;
  SyProfile *d_config;
};


#endif  // LWLOGGER_H
