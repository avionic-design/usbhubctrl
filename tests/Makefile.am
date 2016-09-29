TESTS = check_hub_ctrl

check_PROGRAMS = check_hub_ctrl

check_hub_ctrl_SOURCES = \
	check_hub_ctrl.c \
	check_usb_eeprom.c \
	check_usb_eeprom.h \
	dummy_usb.c \
	dummy_usb.h

check_hub_ctrl_CFLAGS = \
	-I$(top_srcdir)/tests \
	-I$(top_srcdir)/include \
	-DRUN_CHECK \
	$(CHECK_CFLAGS)

check_hub_ctrl_LDADD = \
	$(top_build_prefix)src/lib_eeprom_file_utils.a \
	$(CHECK_LIBS)

LOG_DRIVER = env AM_TAP_AWK='$(AWK)' $(SHELL) \
                  $(top_srcdir)/tap-driver.sh
