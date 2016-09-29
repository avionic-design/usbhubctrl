/**
 * @file
 * @author Meike Vocke <meike.vocke\@avionic-design.de>
 * @date 2016
 *
 * @brief File reading and writing implementation
 *
 * @copyright GPLv3
 */

#ifndef FILE_IO_H
#define FILE_IO_H

#include <stdint.h>
#include <stdio.h>

/**
 * @brief Read data from file into a buffer
 *
 * Opens file and copy read data into buffer. The buffer is allocated inside
 * this function
 *
 * @param file file name, - for stdin as input
 * @param buffer pointer to the buffer pointer
 * @param size_in number of bytes to read from file/stdin, 0 reads the whole file,
 * for stdin the size_in has to be non-zero
 * @return number of actually read bytes on success
 * @return -errno on failure
 */
ssize_t file_read(const char *file, uint8_t **buffer, size_t size_in);

/**
 * @brief Write data from buffer to file
 *
 * Opens file or stdout and wirte the given buffer into the opened file
 * descriptor. If a file was opened and already exists the buffer will be
 * appended to the file. Otherwise the file was created with the given buffer
 * as data inside. In case of stdout the data was written to stdout.
 *
 * @param file file name, - for stdout as output
 * @param buffer pointer to the buffer
 * @param size size of the buffer
 * @return number of actually written bytes to file on success
 * @return -errno on failure
 */
ssize_t file_write(const char *file, const uint8_t *buffer, size_t size);

#endif
