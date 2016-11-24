bin_PROGRAMS = \
	hub-ctrl

hub_ctrl_CFLAGS = \
	@LIBUSB_CFLAGS@ \
	-I$(top_srcdir)/include

hub_ctrl_SOURCES = \
	hub-ctrl.c \
	options.c \
	options.h

hub_ctrl_LDADD = \
	@LIBUSB_LIBS@ \
	$(top_build_prefix)src/lib_eeprom_file_utils.a
