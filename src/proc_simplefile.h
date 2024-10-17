// proc_simplefile.h
//
// Simple File Processor
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

#ifndef PROC_SIMPLEFILE_H
#define PROC_SIMPLEFILE_H

#include "processor.h"

class ProcSimpleFile : public Processor
{
  Q_OBJECT
 public:
  ProcSimpleFile(SyProfile *c,int recv_num,int proc_num,QObject *parent);
  Processor::Type type() const;
  void closeFiles();

 private slots:
  void processMessage(Message *msg,const QHostAddress &from_addr);

 private:
  QString d_base_filename;
  QString d_template;
  FILE *d_base_file;
};


#endif  // PROC_SIMPLEFILE_H
