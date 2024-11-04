// proc_sendmail.h
//
// Processor for sending mail.
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

#ifndef PROC_SENDMAIL_H
#define PROC_SENDMAIL_H

#include "processor.h"

class ProcSendmail : public Processor
{
  Q_OBJECT
 public:
  ProcSendmail(const QString &id,Profile *p,QObject *parent=0);
  Type type() const;
  bool start(QString *err_msg);

 protected:
  void processMessage(Message *msg,const QHostAddress &from_addr);

 private slots:
  void throttleTimeoutData();

 private:
  QString d_from_address;
  QStringList d_to_addresses;
  QString d_subject_line;
  int d_throttle_period;
  int d_throttle_limit;
  int d_throttle_counter;
  QTimer *d_throttle_timer;
};


#endif  // PROCESSOR_H
