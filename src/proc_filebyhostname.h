// proc_filebyhostname.h
//
// Processor for logging to files based on the hostname.
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

#ifndef PROC_FILEBYHOSTNAME_H
#define PROC_FILEBYHOSTNAME_H

#include <QMap>

#include "processor.h"

class ProcFileByHostname : public Processor
{
  Q_OBJECT
 public:
  ProcFileByHostname(const QString &id,Profile *p,QObject *parent=0);
  Processor::Type type() const;
  bool start(QString *err_msg);
  void rotateLogs(const QDateTime &now);

 protected:
  void processMessage(Message *msg,const QHostAddress &from_addr);

 private:
  QDir *d_base_dir;
  QMap<QString,FILE *> d_files;
  QString d_template;
};


#endif  // PROC_FILEBYHOSTNAME_H
