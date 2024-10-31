dnl configure.ac
dnl
dnl Autoconf configuration for lwsyslogger.
dnl Use autoconf to process this into a configure script
dnl
dnl   (C) Copyright 2024 Fred Gleason <fredg@paravelsystems.com>
dnl
dnl   This program is free software; you can redistribute it and/or modify
dnl   it under the terms of the GNU General Public License version 2 as
dnl   published by the Free Software Foundation.
dnl
dnl   This program is distributed in the hope that it will be useful,
dnl   but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl   GNU General Public License for more details.
dnl
dnl   You should have received a copy of the GNU General Public
dnl   License along with this program; if not, write to the Free Software
dnl   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
dnl

AC_INIT([lwsyslogger],m4_esyscmd(cat PACKAGE_VERSION))
AM_INIT_AUTOMAKE([1.9 tar-pax])
AC_SUBST(RPM_RELEASE,1)
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])
AC_CONFIG_MACRO_DIR([m4])

#
# Basic Compiler Checks
#
AC_PROG_CC
AC_PROG_CXX
AC_LANG(C++)

#
# Check for Qt5 (Mandatory)
#
PKG_CHECK_MODULES(QT5_CLI,Qt5Core Qt5Network,,[AC_MSG_ERROR([*** Qt5 not found ***])])
PKG_CHECK_MODULES(QT5_GUI,Qt5Core Qt5Gui Qt5Widgets Qt5Network,,[AC_MSG_ERROR([*** Qt5 not found ***])])
AC_CHECK_PROG(MOC_NAME,moc-qt5,[moc-qt5],[moc])
AC_SUBST(QT_MOC,$MOC_NAME)

AC_CHECK_PROG(LUPDATE_NAME,lupdate-qt5,[lupdate-qt5],[lupdate])
AC_SUBST(QT_LUPDATE,$LUPDATE_NAME)

AC_CHECK_PROG(LRELEASE_NAME,lrelease-qt5,[lrelease-qt5],[lrelease])
AC_SUBST(QT_LRELEASE,$LRELEASE_NAME)

#
# Determine the target architecture
#
AR_GCC_TARGET()
AC_SUBST(VENDOR,$ar_gcc_distro)
AC_SUBST(ARCH,$ar_gcc_arch)

#
# Determine number of available CPU cores for building packages
#
AX_COUNT_CPUS([],[])
if test $CPU_COUNT -lt 2 ; then
   AC_SUBST(CPUS_AVAIL,1)
else
   if test $CPU_COUNT -gt 16 ; then
      AC_SUBST(CPUS_AVAIL,16)
   else
      AC_SUBST(CPUS_AVAIL,$(($CPU_COUNT-1)))
   fi
fi

#
# Determine Distro
#
AR_GET_DISTRO()
AC_SUBST(DISTRO,$ar_gcc_distro)
AC_MSG_NOTICE($ar_distro_id)
DISTRO_NAME="|                         Distribution Name ... Unknown   |"
case "$ar_distro_id" in
     debian)
       AC_MSG_NOTICE([Distro is Debian-ish])
       DISTRO_FAMILY="|                       Distribution Family ... Debian    |"
       DISTRO_IS_DEBIANISH=yes
       DISTRO_NAME="|                         Distribution Name ... Debian    |"
       AC_SUBST(DOC_PATH,"/usr/share/doc/lwsyslogger")
     ;;

     raspbian)
       AC_MSG_NOTICE([Distro is Debian-ish])
       DISTRO_FAMILY="|                       Distribution Family ... Debian    |"
       DISTRO_IS_DEBIANISH=yes
       DISTRO_NAME="|                  Distribution Name ... Raspberry Pi OS  |"
       AC_SUBST(DOC_PATH,"/usr/share/doc/lwsyslogger")
     ;;

     linuxmint)
       AC_MSG_NOTICE([Distro is Debian-ish])
       DISTRO_FAMILY="|                       Distribution Family ... Debian    |"
       DISTRO_IS_DEBIANISH=yes
       DISTRO_NAME="|                         Distribution Name ... Linux Mint|"
       AC_SUBST(DOC_PATH,"/usr/share/doc/lwsyslogger")
     ;;

     ubuntu)
       AC_MSG_NOTICE([Distro is Debian-ish])
       DISTRO_FAMILY="|                       Distribution Family ... Debian    |"
       DISTRO_IS_DEBIANISH=yes
       DISTRO_NAME="|                         Distribution Name ... Ubuntu    |"
       AC_SUBST(DOC_PATH,"/usr/share/doc/lwsyslogger")
     ;;

     centos|fedora|rhel|rocky)
       AC_MSG_NOTICE([Distro is RedHat-ish])
       DISTRO_FAMILY="|                       Distribution Family ... RedHat    |"
       if test $ar_distro_id = "centos" ; then
         DISTRO_NAME="|                         Distribution Name ... CentOS    |"
       fi
       if test $ar_distro_id = "fedora" ; then
         DISTRO_NAME="|                         Distribution Name ... Fedora    |"
       fi
       if test $ar_distro_id = "rhel" ; then
         DISTRO_NAME="|                         Distribution Name ... RHEL      |"
       fi
       if test $ar_distro_id = "rocky" ; then
         DISTRO_NAME="|                         Distribution Name ... Rocky     |"
       fi
       AC_SUBST(DOC_PATH,"/usr/share/doc/lwsyslogger-$PACKAGE_VERSION")
       ;;

     *)
       AC_MSG_NOTICE([Distro is unknown])
       DISTRO_FAMILY="|                       Distribution Family ... Unknown   |"
       AC_SUBST(DOC_PATH,"/usr/share/doc/lwsyslogger")
     ;;
esac

#
# DocBook
#
if test -z $DOCBOOK_DISABLED ; then
  USING_DOCBOOK=yes
fi
AM_CONDITIONAL([DOCBOOK_AM], [test "$USING_DOCBOOK" = yes])
if test $DOCBOOK_STYLESHEETS ; then
   ln -s $DOCBOOK_STYLESHEETS helpers/docbook
fi

#
# Configure RPM Build
#
AC_CHECK_PROG(RPMBUILD_FOUND,rpmbuild,[yes],[])
if test -z $RPMBUILD_FOUND ; then
  AC_SUBST(RPMBUILD,rpm)
else
  AC_SUBST(RPMBUILD,rpmbuild)
fi
if test -d /usr/src/redhat ; then
  AC_SUBST(VENDOR,redhat)
else
  AC_SUBST(VENDOR,suse)
fi 
AC_SUBST(RPM_DATESTAMP,`date +%a\ %b\ %d\ %Y`)

#
# Generated File Warnings
#
AC_SUBST_FILE(GENERATED_C_FILE_WARNING)
GENERATED_C_FILE_WARNING=$srcdir/helpers/c_file_warning

AC_SUBST_FILE(GENERATED_SCRIPT_FILE_WARNING)
GENERATED_SCRIPT_FILE_WARNING=$srcdir/helpers/script_file_warning

AC_SUBST_FILE(GENERATED_XML_FILE_WARNING)
GENERATED_XML_FILE_WARNING=$srcdir/helpers/xml_file_warning

AC_CONFIG_FILES([lwsyslogger.spec \
    conf/Makefile \
    debian/Makefile \
    debian/source/Makefile \
    Makefile \
    helpers/Makefile \
    src/lwsyslogger/Makefile \
    src/tests/Makefile \
    src/Makefile \
    docs/manpages/Makefile \
    docs/Makefile \
    systemd/Makefile \
    systemd/lwsyslogger.service \
    build_debs.sh \
    ])
AC_OUTPUT()

chmod 755 build_debs.sh

#
# Configuration Results
#
AC_MSG_NOTICE()
AC_MSG_NOTICE("|---------------------------------------------------------|")
AC_MSG_NOTICE("|        *** LWSYSLOGGER CONFIGURATION SUMMARY ***        |")
AC_MSG_NOTICE("|---------------------------------------------------------|")
AC_MSG_NOTICE("| Platform Information:                                   |")
AC_MSG_NOTICE("$DISTRO_NAME")
AC_MSG_NOTICE("$DISTRO_FAMILY")
AC_MSG_NOTICE("|---------------------------------------------------------|")
AC_MSG_NOTICE()
AC_MSG_NOTICE()
AC_MSG_NOTICE(Now enter 'make' to build the software.)
AC_MSG_NOTICE()
