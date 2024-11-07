## Makefile.am
##
## lwsyslogger(8) Makefile.am
##
## (C) Copyright 2024 Fred Gleason <fredg@paravelsystems.com>
##
##   This program is free software; you can redistribute it and/or modify
##   it under the terms of the GNU General Public License version 2 as
##   published by the Free Software Foundation.
##
##   This program is distributed in the hope that it will be useful,
##   but WITHOUT ANY WARRANTY; without even the implied warranty of
##   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##   GNU General Public License for more details.
##
##   You should have received a copy of the GNU General Public
##   License along with this program; if not, write to the Free Software
##   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
##
##
## Use automake to process this into a Makefile.in

AM_CPPFLAGS = -Wall -DPREFIX=\"$(prefix)\" -Wno-strict-aliasing -std=c++11 -fPIC -I$(top_srcdir)/lib @QT5_CLI_CFLAGS@
MOC = @QT_MOC@

# The dependency for qt's Meta Object Compiler (moc)
moc_%.cpp:	%.h
	$(MOC) $< -o $@

sbin_PROGRAMS = lwsyslogger

dist_lwsyslogger_SOURCES = addressfilter.cpp addressfilter.h\
                           cmdswitch.cpp cmdswitch.h\
                           local_syslog.h\
                           lwsyslogger.cpp lwsyslogger.h\
                           message.cpp message.h\
                           proc_factory.cpp proc_factory.h\
                           proc_filebyhostname.cpp proc_filebyhostname.h\
                           proc_sendmail.cpp proc_sendmail.h\
                           proc_simplefile.cpp proc_simplefile.h\
                           proc_udp.cpp proc_udp.h\
                           processor.cpp processor.h\
                           profile.cpp profile.h\
                           recv_factory.cpp recv_factory.h\
                           recv_udp.cpp recv_udp.h\
                           receiver.cpp receiver.h\
                           sendmail.cpp sendmail.h

nodist_lwsyslogger_SOURCES = moc_lwsyslogger.cpp\
                             moc_proc_filebyhostname.cpp\
                             moc_proc_sendmail.cpp\
                             moc_proc_simplefile.cpp\
                             moc_proc_udp.cpp\
                             moc_processor.cpp\
                             moc_recv_udp.cpp\
                             moc_receiver.cpp

lwsyslogger_LDADD = @QT5_CLI_LIBS@

CLEANFILES = *~\
             *.idb\
             *ilk\
             *.obj\
             *.pdb\
             moc_*

MAINTAINERCLEANFILES = *~\
                       Makefile.in
