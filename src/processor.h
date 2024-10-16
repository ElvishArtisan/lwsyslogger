// processor.h
//
// Abstract base class for message processors.
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

#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <QDir>
#include <QObject>

#include <sy5/syprofile.h>

#include "message.h"

class Processor : public QObject
{
  Q_OBJECT
 public:
  enum Type {TypeSimpleFile=0,TypeLast=1};
  Processor(SyProfile *c,int recv_num,int proc_num,QObject *parent=0);
  virtual Type type() const=0;
  virtual bool start(QString *err_msg);
  virtual void processMessage(Message *msg,const QHostAddress &from_addr)=0;
  virtual void closeFiles();
  static QString typeString(Type type);
  static Type typeFromString(const QString &str);

 protected:
  SyProfile *config() const;
  int receiverNumber() const;
  int processorNumber() const;
  QDir *logRootDirectory() const;

 private:
  SyProfile *d_config;
  int d_receiver_number;
  int d_processor_number;
  QDir *d_log_root_directory;
};


#endif  // PROCESSOR_H
