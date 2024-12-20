// proc_factory.cpp
//
// Processor Factory
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

#include "proc_filebyhostname.h"
#include "proc_sendmail.h"
#include "proc_simplefile.h"
#include "proc_udp.h"

Processor *ProcessorFactory(Processor::Type type,const QString &id,Profile *p,
			    QObject *parent)
{
  Processor *proc=NULL;
  
  switch(type) {
  case Processor::TypeSimpleFile:
    proc=new ProcSimpleFile(id,p,parent);
    break;

  case Processor::TypeFileByHostname:
    proc=new ProcFileByHostname(id,p,parent);
    break;

  case Processor::TypeSendmail:
    proc=new ProcSendmail(id,p,parent);
    break;

  case Processor::TypeUdp:
    proc=new ProcUdp(id,p,parent);
    break;

  case Processor::TypeLast:
    break;
  }

  return proc;
}
