#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "file_io.h"

#define CHUNK_SIZE 1024

ssize_t file_read(const char *file, uint8_t **buffer, size_t size_in)
{
	uint8_t *l_buffer = NULL;
	struct stat file_stat;
	int read_bytes = 0;
	int ret_val = 0;
	int size = 0;
	int fd;

	if (!file || !buffer)
		return -EINVAL;

	if (!strcmp(file, "-"))
		fd = STDIN_FILENO;
	else
		fd = open(file, O_RDONLY);

	if (fd < 0)
		return -errno;
	else if (fd == STDIN_FILENO && !size_in)
		return -EINVAL;

	if (!size_in) {
		ret_val = fstat(fd, &file_stat);
		if (ret_val != 0)
			return -errno;
		size = file_stat.st_size;
	} else {
		size = size_in;
	}

	if (!size) {
		ret_val = 0;
		goto cleanup;
	}

	l_buffer = malloc(size);
	if (!l_buffer) {
		ret_val = -ENOMEM;
		goto cleanup;
	}

	while (read_bytes < size) {
		ret_val = read(fd, l_buffer + read_bytes, size - read_bytes);
		if (ret_val < 0) {
			ret_val = -errno;
			free(l_buffer);
			goto cleanup;
		}
		read_bytes += ret_val;
		if (!ret_val)
			break;
	}

	ret_val = read_bytes;
	*buffer = l_buffer;

cleanup:
	if (fd > 2)
		close(fd);

	return ret_val;
}

ssize_t file_write(const char *file, const uint8_t *buffer, size_t size)
{
	int ret_val;
	int fd;

	if (!file || !buffer || !size)
		return -EINVAL;

	if (!strcmp(file, "-"))
		fd = STDOUT_FILENO;
	else
		fd = open(file, O_CREAT | O_WRONLY | O_TRUNC, 0600);
	if (fd < 0)
		return -errno;

	ret_val = write(fd, buffer, size);
	if (ret_val < 0)
		ret_val = -errno;

	if (fd > 2)
		close(fd);

	return ret_val;
}
