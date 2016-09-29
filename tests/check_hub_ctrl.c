#include <check.h>
#include <stdio.h>
#include <stdlib.h>

#include "check_usb_eeprom.h"

int main(void)
{
	int number_failed;
	SRunner *sr;
	char filename[] = "-";
	Suite *master_suite;

	master_suite = suite_create("USB_HUB_CTRL");

	sr = srunner_create(master_suite);

	eeprom_suite(master_suite);

	srunner_set_tap(sr, filename);

	srunner_run_all(sr, CK_MINIMAL);

	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);

	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
