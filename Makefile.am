ACLOCAL_AMFLAGS = -I m4

SUBDIRS = \
	bin

EXTRA_DIST = \
	doc/doxyfile.in

MOSTLYCLEANFILES = \
	$(DX_CLEANFILES)

if HAVE_DOXYGEN
@DX_RULES@

all: doxygen-run
endif
