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

#include "addressfilter.h"
#include "profile.h"

#include "message.h"

class Processor : public QObject
{
  Q_OBJECT
 public:
  enum Type {TypeSimpleFile=0,TypeFileByHostname=1,TypeLast=2};
  Processor(const QString &id,Profile *c,QObject *parent=0);
  QString id() const;
  bool overrideTimestamps();
  bool dryRun() const;
  void setDryRun(bool state);
  virtual Type type() const=0;
  virtual bool start(QString *err_msg);
  virtual void rotateLogs(const QDateTime &now);
  static QString typeString(Type type);
  static Type typeFromString(const QString &str);

 public slots:
  void process(Message *msg,const QHostAddress &from_addr);

 protected:
  virtual void processMessage(Message *msg,const QHostAddress &from_addr)=0;
  void rotateLogFile(const QString &filename,const QDateTime &now) const;
  bool expireLogFile(const QString &pathname,const QDateTime &now) const;
  Profile *config() const;
  QDir *logRootDirectory() const;
  void lsyslog(Message::Severity severity,const char *fmt,...) const;

 private slots:
  void logRotationData();
  void deduplicationTimeoutData();
  
 private:
  void StartLogRotationTimer();
  uint32_t MakeSeverityMask(const QString &params,bool *ok,
			    QString *err_msg) const;
  uint32_t MakeFacilityMask(const QString &params,bool *ok,
			    QString *err_msg) const;
  uint32_t MakeMask(uint32_t num) const;
  uint32_t d_facility_mask;
  uint32_t d_severity_mask;
  AddressFilter *d_address_filter;
  Profile *d_profile;
  QTimer *d_log_rotation_timer;
  QTime d_log_rotation_time;
  int d_old_log_purge_age;
  int d_log_rotation_age;
  int d_log_rotation_size;
  QString d_id;
  bool d_dry_run;
  int d_deduplication_timeout;
  QTimer *d_deduplication_timer;
  Message d_last_message;
  int d_last_message_count;
  bool d_override_timestamps;
  QDir *d_log_root_directory;
};


#endif  // PROCESSOR_H
