#include <check.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "file_io.h"

char file_name[] = "/tmp/fileXXXXXX";
uint8_t *file_buffer = NULL;
const uint8_t cmp_buffer[] = {
	0xD4, 0xB4, 0x04, 0x60, 0x65, 0x00, 0x92, 0x88,
	0x28, 0x5F, 0x00, 0x00, 0x50, 0xBE, 0x50, 0x64,
	0x32, 0x80, 0x61, 0x00, 0x02, 0x07, 0x0F, 0x0F,
	0x09, 0x04, 0x04, 0x04, 0x28, 0x00, 0x34, 0x00,
	0x4E, 0x00, 0x6E, 0x00, 0x82, 0x00, 0x82, 0x00,
	0x0C, 0x03, 0x43, 0x00, 0x31, 0x00, 0x30, 0x00,
	0x30, 0x00, 0x38, 0x00, 0x1A, 0x03, 0xFF, 0xFE,
	0xCF, 0x67, 0xEB, 0x58, 0x4A, 0x53, 0x0E, 0x5C,
	0xD4, 0x9A, 0xA1, 0x80, 0xFD, 0x4E, 0x09, 0x67,
	0x50, 0x96, 0x6C, 0x51, 0xF8, 0x53, 0x20, 0x03,
	0x41, 0x00, 0x44, 0x00, 0x54, 0x00, 0x31, 0x00,
	0x38, 0x00, 0x31, 0x00, 0x30, 0x00, 0x2D, 0x00,
	0x30, 0x00, 0x31, 0x00, 0x33, 0x00, 0x2D, 0x00,
	0x30, 0x00, 0x30, 0x00, 0x31, 0x00, 0x14, 0x03,
	0x55, 0x53, 0x00, 0x4E, 0xB3, 0x50, 0x38, 0x8F,
	0x49, 0x8F, 0x6F, 0x8B, 0xC6, 0x96, 0xBF, 0x7E,
	0x68, 0x56, 0x0E, 0x03, 0x30, 0x00, 0x30, 0x00,
	0x30, 0x00, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00
};

void setup_create_file_with_data()
{
	int ret_val = 0;
	int fd;

	fd = mkstemp(file_name);
	ck_assert_int_ge(fd, 0);

	ret_val = write(fd, cmp_buffer, sizeof(cmp_buffer));
	ck_assert_int_eq(ret_val, sizeof(cmp_buffer));

	close(fd);
}

void setup_tmpfile_malloc()
{
	int fd = 0;

	fd = mkstemp(file_name);
	ck_assert_int_ge(fd, 0);

	file_buffer = malloc(sizeof(cmp_buffer));
	ck_assert_ptr_ne(file_buffer, NULL);

	close(fd);
}

void teardown()
{
	int ret_val = 0;

	ret_val = remove(file_name);
	ck_assert_int_eq(ret_val, 0);

	free(file_buffer);
	strcpy(file_name, "/tmp/fileXXXXXX");
}

START_TEST(test_file_read_boundaries)
{
	ssize_t ret_val = 0;

	ret_val = file_read(NULL, &file_buffer, sizeof(cmp_buffer));
	ck_assert_int_eq(ret_val, -EINVAL);
}
END_TEST

START_TEST(test_file_read)
{
	ssize_t ret_val = 0;

	/* read cmp_size bytes from file */
	ret_val = file_read(file_name, &file_buffer, sizeof(cmp_buffer));
	ck_assert_ptr_ne(file_buffer, NULL);
	ck_assert_int_eq(ret_val, sizeof(cmp_buffer));

	ck_assert_int_eq(memcmp(file_buffer, cmp_buffer, sizeof(cmp_buffer)), 0);

	/* read all bytes from file */
	ret_val = file_read(file_name, &file_buffer, 0);
	ck_assert_ptr_ne(file_buffer, NULL);
	ck_assert_int_eq(ret_val, sizeof(cmp_buffer));

	ck_assert_int_eq(memcmp(file_buffer, cmp_buffer, sizeof(cmp_buffer)), 0);
}
END_TEST

START_TEST(test_file_write_boundaries)
{
	int ret_val = 0;

	/* missing filename */
	ret_val = file_write(NULL, cmp_buffer, sizeof(cmp_buffer));
	ck_assert_int_eq(ret_val, -EINVAL);

	/* missing size */
	ret_val = file_write(file_name, cmp_buffer, 0);
	ck_assert_int_eq(ret_val, -EINVAL);

	/* missing buffer */
	ret_val = file_write(file_name, NULL, sizeof(cmp_buffer));
	ck_assert_int_eq(ret_val, -EINVAL);
}
END_TEST

START_TEST(test_file_write)
{
	int ret_val = 0;
	int size = 0;
	int fd;

	size = file_write(file_name, cmp_buffer, sizeof(cmp_buffer));
	ck_assert_int_eq(size, sizeof(cmp_buffer));

	fd = open(file_name, O_RDONLY);
	ck_assert_int_ge(fd, 0);

	ret_val = read(fd, file_buffer, sizeof(cmp_buffer));
	ck_assert_int_eq(ret_val, sizeof(cmp_buffer));

	ck_assert_int_eq(memcmp(file_buffer, cmp_buffer, sizeof(cmp_buffer)), 0);

	close(fd);
}
END_TEST

Suite * file_io_suite(Suite *s_file)
{
	TCase *tc_file_write;
	TCase *tc_file_read;

	tc_file_read = tcase_create("file read");
	tc_file_write = tcase_create("file write");

	tcase_add_unchecked_fixture(tc_file_read, setup_create_file_with_data, teardown);
	tcase_add_test(tc_file_read, test_file_read);
	tcase_add_test(tc_file_read, test_file_read_boundaries);

	tcase_add_unchecked_fixture(tc_file_write, setup_tmpfile_malloc,
			teardown);
	tcase_add_test(tc_file_write, test_file_write);
	tcase_add_test(tc_file_write, test_file_write_boundaries);

	suite_add_tcase(s_file, tc_file_read);
	suite_add_tcase(s_file, tc_file_write);

	return EXIT_SUCCESS;
}
