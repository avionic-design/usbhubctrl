ACLOCAL_AMFLAGS = -I m4

SUBDIRS = \
	src \
	bin

include_HEADERS = \
	include/file_io.h \
	include/usb_eeprom.h

EXTRA_DIST = \
	doc/doxyfile.in

MOSTLYCLEANFILES = \
	$(DX_CLEANFILES)

if HAVE_DOXYGEN
@DX_RULES@

all: doxygen-run
endif
