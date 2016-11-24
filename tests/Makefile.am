TESTS = check_hub_ctrl

check_LIBRARIES = libusb_mock.a

libusb_mock_a_CFLAGS = \
	-I$(top_srcdir)/tests \
	@LIBUSB_CFLAGS@

libusb_mock_a_SOURCES = \
	dummy_usb.c

check_PROGRAMS = check_hub_ctrl

check_hub_ctrl_SOURCES = \
	check_file_io.c \
	check_file_io.h \
	check_hub_ctrl.c \
	check_usb_eeprom.c \
	check_usb_eeprom.h \
	check_usb_eeprom_data.h \
	dummy_usb.c \
	dummy_usb.h

check_hub_ctrl_CFLAGS = \
	-I$(top_srcdir)/tests \
	-I$(top_srcdir)/include \
	-DRUN_CHECK \
	$(CHECK_CFLAGS) \
	@LIBUSB_CFLAGS@

check_hub_ctrl_LDADD = \
	$(top_build_prefix)src/lib_eeprom_file_utils.a \
	$(top_build_prefix)tests/libusb_mock.a \
	$(CHECK_LIBS)

LOG_DRIVER = env AM_TAP_AWK='$(AWK)' $(SHELL) \
                  $(top_srcdir)/tap-driver.sh
