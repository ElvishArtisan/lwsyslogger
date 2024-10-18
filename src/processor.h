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

#include <QDateTime>
#include <QDir>
#include <QObject>
#include <QTimer>

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
  void process(Message *msg,const QHostAddress &from_addr);
  QString idString() const;
  virtual void rotateLogs(const QDateTime &now);
  static QString typeString(Type type);
  static Type typeFromString(const QString &str);

 protected:
  virtual void processMessage(Message *msg,const QHostAddress &from_addr)=0;
  void rotateLogFile(const QString &filename,const QDateTime &now) const;
  bool expireLogFile(const QString &pathname,const QDateTime &now) const;
  SyProfile *config() const;
  int receiverNumber() const;
  int processorNumber() const;
  bool processIf(Message::Facility facility) const;
  bool processIf(Message::Severity severity) const;
  QDir *logRootDirectory() const;

 private slots:
  void logRotationData();
  
 private:
  void StartLogRotationTimer();
  uint32_t MakeSeverityMask(const QString &params,bool *ok,
			    QString *err_msg) const;
  uint32_t MakeFacilityMask(const QString &params,bool *ok,
			    QString *err_msg) const;
  uint32_t MakeMask(uint32_t num) const;
  uint32_t d_facility_mask;
  uint32_t d_severity_mask;
  SyProfile *d_config;
  QTimer *d_log_rotation_timer;
  QTime d_log_rotation_time;
  int d_old_log_purge_age;
  int d_log_rotation_age;
  int d_log_rotation_size;
  int d_receiver_number;
  int d_processor_number;
  QDir *d_log_root_directory;
};


#endif  // PROCESSOR_H
