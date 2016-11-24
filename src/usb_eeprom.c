#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <libusb.h>

#include "usb_eeprom.h"

#define CYPRESS_HUB_VID		0x04b4
#define CYPRESS_HUB_PID		0x6560

int usb_eeprom_erase(libusb_device_handle *dev, size_t size)
{
	uint8_t *erase_buf;
	int ret_val = 0;

	if (!dev || !size)
		return -EINVAL;
	if (size > MAX_EEPROM_SIZE)
		return -ERANGE;

	erase_buf = malloc(size);
	if (!erase_buf)
		return -ENOMEM;
	memset(erase_buf, 0xff, size);

	ret_val = usb_eeprom_write(dev, erase_buf, size);

	free(erase_buf);

	return ret_val;
}

int usb_eeprom_read(libusb_device_handle *dev, uint8_t *buffer, size_t size)
{
	int len;

	if (!buffer || !dev || !size)
		return -EINVAL;
	if (size > MAX_EEPROM_SIZE)
		return -ERANGE;

	len = libusb_control_transfer(dev, USB_REQ_TYPE_READ_EEPROM,
		USB_REQ_READ, 0, 0, buffer, size, GET_TIMEOUT(size));

	return len;
}

int usb_eeprom_write(libusb_device_handle *dev, uint8_t *buffer, size_t size)
{
	int len;

	if (!buffer || !dev || !size)
		return -EINVAL;
	if (size > MAX_EEPROM_SIZE)
		return -ERANGE;

	len = libusb_control_transfer(dev, USB_REQ_TYPE_WRITE_EEPROM,
		USB_REQ_WRITE, 0, 0, buffer, size, GET_TIMEOUT(size));

	/*
	 * Sleep for more than 5 ms to guarantee EEPROM data is written.
	 * This is the maximal write cycle time of the EEPROM, specified
	 * in the 25AA640/25LC640 datasheet.
	 */
	usleep(5500);

	return len;
}

int usb_eeprom_support(libusb_device *dev)
{
	struct libusb_device_descriptor desc;
	int ret;

	if (!dev)
		return -EINVAL;

	ret = libusb_get_device_descriptor(dev, &desc);
	if (ret)
		return -EIO;

	if (desc.idVendor != CYPRESS_HUB_VID ||
			desc.idProduct != CYPRESS_HUB_PID)
		return ret;

	ret |= EEPROM_SUPPORT_DEVICE;

	if (desc.bDeviceClass == LIBUSB_CLASS_HUB && desc.bcdDevice != 0x9015)
		ret |= EEPROM_SUPPORT_STORAGE;
	else if (desc.bDeviceClass == LIBUSB_CLASS_VENDOR_SPEC &&
			desc.bcdDevice == 0x9015)
		ret |= EEPROM_SUPPORT_STORAGE | EEPROM_SUPPORT_BLANK;

	return ret;
}
