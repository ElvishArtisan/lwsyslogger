## Makefile.am
##
## docs/manpages/Makefile.am for lwsyslogger
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
## Use automake to process this into a Makefile.in

##
## Build Dependencies
##
%.html:	%.xml
	xsltproc -o $@ $(DOCBOOK_STYLESHEETS)/xhtml/docbook.xsl $<
%.pdf:	%.xml
	xsltproc $(DOCBOOK_STYLESHEETS)/fo/docbook.xsl $< | fop - -pdf $@
%.1:	%.xml
	xsltproc $(DOCBOOK_STYLESHEETS)/manpages/docbook.xsl $<
%.5:	%.xml
	xsltproc $(DOCBOOK_STYLESHEETS)/manpages/docbook.xsl $<
%.7:	%.xml
	xsltproc $(DOCBOOK_STYLESHEETS)/manpages/docbook.xsl $<
%.8:	%.xml
	xsltproc $(DOCBOOK_STYLESHEETS)/manpages/docbook.xsl $<

all-local:	lwsyslogger.8\
		lwsyslogger.conf.5

man_MANS = lwsyslogger.8\
           lwsyslogger.conf.5

EXTRA_DIST = lwsyslogger.xml\
             lwsyslogger.8\
             lwsyslogger.conf.xml\
             lwsyslogger.conf.5

CLEANFILES = *~
MAINTAINERCLEANFILES = *~\
                       *.1\
                       *.5\
                       *.5\
                       *.7\
                       *.8\
                       *.html\
                       *.pdf\
                       aclocal.m4\
                       configure\
                       Makefile.in
