/**
 * @file
 * @author Meike Vocke <meike.vocke\@avionic-design.de>
 * @date 2016
 *
 * @brief Provide testsuite for file_io
 *
 * @copyright GPLv3
 */

#ifndef CHECK_RW_FILE_H
#define CHECK_RW_FILE_H

/**
 * @brief Add file io test cases to the given suite
 *
 * @return file_suite Suite the test cases should be added
 * @return 0 on success
 */
int file_io_suite(Suite *file_suite);

#endif /* CHECK_RW_FILE_H */
