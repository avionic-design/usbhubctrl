#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "usb_eeprom.h"

int usb_eeprom_erase(usb_dev_handle *uh, size_t size)
{
	uint8_t *erase_buf;
	int len;
	if (!uh || !size)
		return -EINVAL;
	if (size > MAX_EEPROM_SIZE)
		return -ERANGE;

	erase_buf = malloc(size);
	if (!erase_buf)
		return -ENOMEM;
	memset(erase_buf, 0xff, size);

	len = usb_control_msg(uh, USB_REQ_TYPE_WRITE_EEPROM,
			USB_REQ_WRITE, 0, 0, (char *)erase_buf, size,
			CTRL_TIMEOUT);

	free(erase_buf);

	return len;
}

int usb_eeprom_read(usb_dev_handle *uh, uint8_t *buffer, size_t size)
{
	int len;

	if (!buffer || !uh || !size)
		return -EINVAL;
	if (size > MAX_EEPROM_SIZE)
		return -ERANGE;

	len = usb_control_msg(uh, USB_REQ_TYPE_READ_EEPROM, USB_REQ_READ, 0,
			0, (char *)buffer, size, CTRL_TIMEOUT);

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
			0, 0, (char *)buffer, size, CTRL_TIMEOUT);

	return len;
}
