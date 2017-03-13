AC_PREREQ(2.66)
AC_INIT([usbhubctrl], [0.6.1], [bugs@avionic-design.de], [usbhubctrl])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([Makefile.am])
AC_CONFIG_HEADER([config.h])
AC_CANONICAL_HOST
AC_REQUIRE_AUX_FILE([tap-driver.sh])

AM_INIT_AUTOMAKE([no-dist-gzip dist-xz foreign])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
AM_MAINTAINER_MODE

AC_PROG_LIBTOOL
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_INSTALL
AC_PROG_AWK

DX_INIT_DOXYGEN([$PACKAGE_NAME], [doc/doxyfile], [doc/doxygen])
AM_CONDITIONAL([HAVE_DOXYGEN],[test -n "${DX_DOXYGEN}"])

PKG_CHECK_MODULES(LIBUSB, libusb-1.0)
AC_CHECK_HEADER([libusb-1.0/libusb.h])

PKG_CHECK_MODULES([CHECK], [check >= 0.9.4],
		[have_check=yes], [have_check=no])
AS_IF(test "x$have_check" = xyes)
	AC_CHECK_HEADER([check.h])
AM_CONDITIONAL([BUILD_TEST], [test "x$have_check" = xyes])

AC_ARG_WITH([udev-dir],
	AS_HELP_STRING([--with-udev-dir=DIR],
		[Directory where udev expects its rules files
		[default=${libdir}/udev/rules.d]]),
	[udevdir="$withval"],
	[udevdir="${libdir}/udev/rules.d"])
UDEV_RULES_DIR=${udevdir}
AC_SUBST(UDEV_RULES_DIR)

CFLAGS="$CFLAGS -Wall -Werror"

AC_CONFIG_FILES([
	Makefile
	src/Makefile
	bin/Makefile
	doc/doxyfile
	tests/Makefile
	udev/Makefile
])

AC_OUTPUT
