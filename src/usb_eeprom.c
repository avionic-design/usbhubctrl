#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>

#include "usb_eeprom.h"

int usb_eeprom_erase(usb_dev_handle *uh, size_t size)
{
	uint8_t *erase_buf;
	int ret_val = 0;

	if (!uh || !size)
		return -EINVAL;
	if (size > MAX_EEPROM_SIZE)
		return -ERANGE;

	erase_buf = malloc(size);
	if (!erase_buf)
		return -ENOMEM;
	memset(erase_buf, 0xff, size);

	ret_val = usb_eeprom_write(uh, erase_buf, size);

	free(erase_buf);

	return ret_val;
}

int usb_eeprom_read(usb_dev_handle *uh, uint8_t *buffer, size_t size)
{
	int len;

	if (!buffer || !uh || !size)
		return -EINVAL;
	if (size > MAX_EEPROM_SIZE)
		return -ERANGE;

	len = usb_control_msg(uh, USB_REQ_TYPE_READ_EEPROM, USB_REQ_READ, 0,
			0, (char *)buffer, size, GET_TIMEOUT(size));

	return len;
}

int usb_eeprom_write(usb_dev_handle *uh, uint8_t *buffer, size_t size)
{
	int len;

	if (!buffer || !uh || !size)
		return -EINVAL;
	if (size > MAX_EEPROM_SIZE)
		return -ERANGE;

	len = usb_control_msg(uh, USB_REQ_TYPE_WRITE_EEPROM, USB_REQ_WRITE,
			0, 0, (char *)buffer, size, GET_TIMEOUT(size));

	/*
	 * Sleep for more than 5 ms to guarantee EEPROM data is written.
	 * This is the maximal write cycle time of the EEPROM, specified
	 * in the 25AA640/25LC640 datasheet.
	 */
	usleep(5500);

	return len;
}
