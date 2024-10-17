// local_syslog.h
//
// Local syslog implementation for lwsyslogger.
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

#ifndef LOCAL_SYSLOG_H
#define LOCAL_SYSLOG_H

#include <QString>

#include "message.h"

extern bool debug;
extern void LocalSyslog(Message::Severity severity,const char *format,...);


#endif  // LOCAL_SYSLOG_H
