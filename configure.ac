AC_PREREQ(2.66)
AC_INIT([usbhubctrl], [0.1.0], [bugs@avionic-design.de], [usbhubctrl])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([Makefile.am])
AC_CONFIG_HEADER([config.h])
AC_CANONICAL_HOST

AM_INIT_AUTOMAKE([no-dist-gzip dist-xz foreign])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
AM_MAINTAINER_MODE

AC_PROG_LIBTOOL
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_INSTALL

PKG_CHECK_MODULES(LIBUSB, libusb)
AC_CHECK_HEADER([usb.h])

CFLAGS="$CFLAGS -Wall -Werror"

AC_OUTPUT([
	Makefile
])
