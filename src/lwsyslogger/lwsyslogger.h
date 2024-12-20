// lwsyslogger.h
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

#ifndef LWSYSLOGGER_H
#define LWSYSLOGGER_H

#include <QDir>
#include <QMap>
#include <QObject>
#include <QTimer>

#include "local_syslog.h"
#include "processor.h"
#include "profile.h"
#include "receiver.h"

//
// Defaults
//
#define DEFAULT_CONFIG_FILENAME "/etc/lwsyslogger.conf"
#define DEFAULT_LOGROOT "/var/log/lwsyslogger"
#define DEFAULT_SERVICE_USER "lwsyslogger"
#define DEFAULT_SERVICE_GROUP "lwsyslogger"

//
// Global RIPCD Definitions
//
#define LWSYSLOGGER_USAGE "[--config=<filename>] [--rotate-logfiles[=YYYY-MM-DD]] [--dry-run] [--no-local-syslog] [-d]\n\n"

class MainObject : public QObject
{
  Q_OBJECT
 public:
  MainObject(QObject *parent=0);

 private slots:
  void exitData();
   
 private:
  bool ConfigureLogRoot(QString *err_msg);
  bool StartProcessors(QString *err_msg,bool dry_run);
  bool StartReceivers(QString *err_msg);
  QMap<QString,Receiver *> d_receivers;
  QMap<QString,Processor *> d_processors;
  QDir *d_logroot_dir;
  uid_t d_uid;
  gid_t d_gid;
  QString d_user_name;
  QString d_group_name;
  Profile *d_profile;
  QTimer *d_exit_timer;
  friend void LocalSyslog(int prio,const QString &msg);
};


#endif  // LWSYSLOGGER_H
