## Makefile.am
##
## Top level automake.am for lwlogger
##
##   (C) Copyright 2024 Fred Gleason <fredg@paravelsystems.com>
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

SUBDIRS = debian\
          docs\
          helpers\
          src\
          systemd

ACLOCAL_AMFLAGS = -I m4

maintainer-clean-local:	
	rm -rf m4

deb:	dist
	./build_debs.sh

rpm:	dist
	mkdir -p $(HOME)/rpmbuild/SOURCES
	cp lwlogger-$(VERSION).tar.gz $(HOME)/rpmbuild/SOURCES/
	$(RPMBUILD) -ba --target $(ARCH)-$(VENDOR)-linux lwlogger.spec
	mv $(HOME)/rpmbuild/RPMS/$(ARCH)/lwlogger-*.rpm $(top_srcdir)/
	mv $(HOME)/rpmbuild/RPMS/noarch/lwlogger-*.rpm $(top_srcdir)/
	mv $(HOME)/rpmbuild/SRPMS/lwlogger-*.src.rpm $(top_srcdir)/
	rm $(HOME)/rpmbuild/SOURCES/lwlogger-$(VERSION).tar.gz

EXTRA_DIST = autogen.sh\
             CODINGSTYLE\
             configure_build.sh\
             get_distro.pl\
             get_target.sh\
             INSTALL-WINDOWS\
             lwlogger.ism\
             lwlogger.pro\
             lwlogger.spec.in\
             UPGRADING

CLEANFILES = *~\
             *.buildinfo\
             *.changes\
             *.ddeb\
             *.deb\
             *.dsc\
             *.mo\
             *.rpm\
             *.tar.gz\
             *.tar.xz\
             *.tgz\
             *.zip\
             py-compile

DISTCLEANFILES = *.spec\
                 config.guess\
                 config.sub\
                 ltmain.sh\
                 lwlogger

MAINTAINERCLEANFILES = *~\
                       *.tar.gz\
                       aclocal.m4\
                       Makefile.in\
                       compile\
                       config.guess\
                       config.status\
                       config.sub\
                       configure\
                       depcomp\
                       install-sh\
                       libtool\
                       ltmain.sh\
                       missing\
                       mkinstalldirs\
                       lwlogger.spec
