#include <check.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "dummy_usb.h"
#include "usb_eeprom.h"
#include "check_usb_eeprom_data.h"

libusb_device_handle *uh = NULL;
uint8_t eeprom_buffer[] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
	0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
	0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
	0x30, 0x31
};

void setup_device_handle()
{
	uh = libusb_device_handle_create();
	ck_assert_ptr_ne(uh, NULL);
}

void setup_device_handle_with_malloc()
{
	uh = libusb_device_handle_create();
	ck_assert_ptr_ne(uh, NULL);
}

void setup_device_handle_with_malloc_init()
{
	uh = libusb_device_handle_create();
	ck_assert_ptr_ne(uh, NULL);
}

void teardown_device_handle()
{
	if (uh)
		libusb_device_handle_free(&uh);
}

void teardown_device_handle_with_buffer()
{
	if (uh)
		libusb_device_handle_free(&uh);
}

START_TEST(test_eeprom_erase_boundaries)
{
	int ret_val = 0;

	/* check for invalid size */
	ret_val = usb_eeprom_erase(uh, MAX_EEPROM_SIZE + 1);
	ck_assert_int_eq(ret_val, -ERANGE);

	/* check for invalid size */
	ret_val = usb_eeprom_erase(uh, 0);
	ck_assert_int_eq(ret_val, -EINVAL);

	/* check for libusb_device_handle uninitialised */
	ret_val = usb_eeprom_erase(NULL, sizeof(eeprom_buffer));
	ck_assert_int_eq(ret_val, -EINVAL);
}
END_TEST

START_TEST(test_eeprom_erase)
{
	struct usb_msg *msg = NULL;
	int ret_val = 0;
	int i;

	ret_val = usb_eeprom_erase(uh, sizeof(eeprom_buffer));
	ck_assert_int_eq(ret_val, sizeof(eeprom_buffer));

	msg = get_usb_msg(uh);
	ck_assert_ptr_ne(msg, NULL);
	ck_assert_int_eq(msg->requesttype, USB_REQ_TYPE_WRITE_EEPROM);
	ck_assert_int_eq(msg->request, USB_REQ_WRITE);
	ck_assert_int_eq(msg->value, 0);
	ck_assert_int_eq(msg->index, 0);
	ck_assert_ptr_ne(msg->bytes, NULL);
	ck_assert_int_eq(msg->size, sizeof(eeprom_buffer));
	ck_assert_int_eq(msg->timeout, GET_TIMEOUT(sizeof(eeprom_buffer)));

	for (i = 0; i < sizeof(eeprom_buffer); i++)
		ck_assert_uint_eq(msg->bytes[i], 0xff);
}
END_TEST

START_TEST(test_eeprom_write_boundaries)
{
	int ret_val = 0;

	/* size is zero */
	ret_val = usb_eeprom_write(uh, eeprom_buffer, 0);
	ck_assert_int_eq(ret_val, -EINVAL);

	/* size to large */
	ret_val = usb_eeprom_write(uh, eeprom_buffer, MAX_EEPROM_SIZE + 1);
	ck_assert_int_eq(ret_val, -ERANGE);

	/* missing buffer */
	ret_val = usb_eeprom_write(uh, NULL, sizeof(eeprom_buffer));
	ck_assert_int_eq(ret_val, -EINVAL);

	/* missing usb_device_handle */
	ret_val = usb_eeprom_write(NULL, eeprom_buffer, sizeof(eeprom_buffer));
	ck_assert_int_eq(ret_val, -EINVAL);
}
END_TEST

START_TEST(test_eeprom_write)
{
	struct usb_msg *msg = NULL;
	int ret_val = 0;

	ret_val = usb_eeprom_write(uh, eeprom_buffer, sizeof(eeprom_buffer));
	ck_assert_int_eq(ret_val, sizeof(eeprom_buffer));

	msg = get_usb_msg(uh);
	ck_assert_ptr_ne(msg, NULL);
	ck_assert_int_eq(msg->requesttype, USB_REQ_TYPE_WRITE_EEPROM);
	ck_assert_int_eq(msg->request, USB_REQ_WRITE);
	ck_assert_int_eq(msg->value, 0);
	ck_assert_int_eq(msg->index, 0);
	ck_assert_ptr_ne(msg->bytes, NULL);
	ck_assert_int_eq(msg->size, sizeof(eeprom_buffer));
	ck_assert_int_eq(msg->timeout, GET_TIMEOUT(sizeof(eeprom_buffer)));

	ck_assert_int_eq(memcmp(msg->bytes, eeprom_buffer,
			sizeof(eeprom_buffer)), 0);
}
END_TEST

START_TEST(test_eeprom_read_boundaries)
{
	int ret_val = 0;

	/* size is zero */
	ret_val = usb_eeprom_read(uh, eeprom_buffer, 0);
	ck_assert_int_eq(ret_val, -EINVAL);

	/* size to large */
	ret_val = usb_eeprom_read(uh, eeprom_buffer, MAX_EEPROM_SIZE + 1);
	ck_assert_int_eq(ret_val, -ERANGE);

	/* missing buffer */
	ret_val = usb_eeprom_read(uh, NULL, sizeof(eeprom_buffer));
	ck_assert_int_eq(ret_val, -EINVAL);

	/* missing usb_device_handle */
	ret_val = usb_eeprom_read(NULL, eeprom_buffer, sizeof(eeprom_buffer));
	ck_assert_int_eq(ret_val, -EINVAL);
}
END_TEST

START_TEST(test_eeprom_read)
{
	struct usb_msg *msg = NULL;
	int ret_val = 0;

	ret_val = usb_eeprom_read(uh, eeprom_buffer, sizeof(eeprom_buffer));
	ck_assert_int_eq(ret_val, sizeof(eeprom_buffer));

	msg = get_usb_msg(uh);
	ck_assert_ptr_ne(msg, NULL);
	ck_assert_int_eq(msg->requesttype, USB_REQ_TYPE_READ_EEPROM);
	ck_assert_int_eq(msg->request, USB_REQ_READ);
	ck_assert_int_eq(msg->value, 0);
	ck_assert_int_eq(msg->index, 0);
	ck_assert_ptr_ne(msg->bytes, NULL);
	ck_assert_int_eq(msg->size, sizeof(eeprom_buffer));
	ck_assert_int_eq(msg->timeout, GET_TIMEOUT(sizeof(eeprom_buffer)));
}
END_TEST

/**
 * @test usb_eeprom_support()
 */
START_TEST(test_eeprom_support)
{
	struct data_eesupport *data = &eeprom_support_data[_i];
	int ret;

	ret = usb_eeprom_support((libusb_device *)data);
	ck_assert_int_eq(ret, data->ret);
}
END_TEST

/**
 * @test usb_eeprom_support()
 */
START_TEST(test_eeprom_support_boundaries)
{
	ck_assert_int_eq(usb_eeprom_support(NULL), -EINVAL);
}
END_TEST

int eeprom_suite(Suite *s_eeprom)
{
	TCase *tc_eeprom_erase;
	TCase *tc_eeprom_write;
	TCase *tc_eeprom_read;
	TCase *tc_eeprom_support;

	tc_eeprom_erase = tcase_create("EEPROM erase");
	tc_eeprom_write = tcase_create("EEPROM write");
	tc_eeprom_read = tcase_create("EEPROM read");
	tc_eeprom_support = tcase_create("EEPROM support");

	tcase_add_unchecked_fixture(tc_eeprom_erase, setup_device_handle,
			teardown_device_handle);
	tcase_add_test(tc_eeprom_erase, test_eeprom_erase);
	tcase_add_test(tc_eeprom_erase, test_eeprom_erase_boundaries);

	tcase_add_unchecked_fixture(tc_eeprom_write,
			setup_device_handle_with_malloc_init,
			teardown_device_handle_with_buffer);
	tcase_add_test(tc_eeprom_write, test_eeprom_write);
	tcase_add_test(tc_eeprom_write, test_eeprom_write_boundaries);

	tcase_add_unchecked_fixture(tc_eeprom_read,
			setup_device_handle_with_malloc,
			teardown_device_handle_with_buffer);
	tcase_add_test(tc_eeprom_read, test_eeprom_read);
	tcase_add_test(tc_eeprom_read, test_eeprom_read_boundaries);

	tcase_add_loop_test(tc_eeprom_support, test_eeprom_support, 0,
		sizeof(eeprom_support_data) / sizeof(struct data_eesupport));
	tcase_add_test(tc_eeprom_support, test_eeprom_support_boundaries);

	suite_add_tcase(s_eeprom, tc_eeprom_erase);
	suite_add_tcase(s_eeprom, tc_eeprom_write);
	suite_add_tcase(s_eeprom, tc_eeprom_read);
	suite_add_tcase(s_eeprom, tc_eeprom_support);

	return EXIT_SUCCESS;
}
