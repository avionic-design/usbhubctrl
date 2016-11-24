#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "dummy_usb.h"

/** USB control message Requesttype for EEPROM read */
#define USB_REQ_TYPE_READ_EEPROM	0xC0
/** USB control message Requesttype for EEPROM write */
#define USB_REQ_TYPE_WRITE_EEPROM	0x40
/** USB control message Request for read */
#define USB_REQ_READ			0x02
/** USB control message Request for write */
#define USB_REQ_WRITE			0x01

libusb_device_handle *libusb_device_handle_create()
{
	libusb_device_handle *uh = NULL;

	uh = malloc(sizeof(libusb_device_handle));
	if (!uh)
		return NULL;

	uh->msg = calloc(1, sizeof(struct usb_msg));
	if (!(uh->msg)) {
		free(uh);
		return NULL;
	}

	return uh;
}

int libusb_device_handle_free(libusb_device_handle **dev)
{
	if (!(*dev) || !((*dev)->msg))
		return -EINVAL;

	if ((*dev)->msg->bytes) {
		free((*dev)->msg->bytes);
		(*dev)->msg->bytes = NULL;
	}

	free((*dev)->msg);
	(*dev)->msg = NULL;
	free(*dev);
	*dev = NULL;

	return 0;
}

struct usb_msg *get_usb_msg(libusb_device_handle *dev)
{
	if (!dev)
		return NULL;

	return dev->msg;
}

/* declared in libusb.h */
int libusb_control_transfer(libusb_device_handle *dev_handle,
	uint8_t request_type, uint8_t bRequest, uint16_t wValue, uint16_t wIndex,
	unsigned char *data, uint16_t wLength, unsigned int timeout)
{
	if (!dev_handle || !dev_handle->msg || !data || !wLength)
		return -EINVAL;

	if (request_type != USB_REQ_TYPE_READ_EEPROM &&
			request_type != USB_REQ_TYPE_WRITE_EEPROM)
		return -ERANGE;

	if (bRequest != USB_REQ_READ && bRequest != USB_REQ_WRITE)
		return -ERANGE;

	dev_handle->msg->bytes = malloc(wLength);
	if (!dev_handle->msg->bytes)
		return -ENOMEM;

	dev_handle->msg->requesttype = request_type;
	dev_handle->msg->request = bRequest;
	dev_handle->msg->value = wValue;
	dev_handle->msg->index = wIndex;
	memcpy(dev_handle->msg->bytes, data, wLength);
	dev_handle->msg->size = wLength;
	dev_handle->msg->timeout = timeout;

	return wLength;
}

/**
 * @brief Mock for reading the device descriptor
 *
 * Mock function to imitate fetching a descriptor for a USB device. The results
 * shall be supplied via the first argument.
 *
 * @param dev Casted pointer to the desired actual descriptor
 * @param desc Pointer to the resulting descriptor
 * @return 0 on success
 * @return -EINVAL if any arguments are NULL
 * @pre dev @b must be allocated to memory at least the size of the descriptor!
 * An easy and safe way to achieve this is casting pointers around.
 */
int libusb_get_device_descriptor(libusb_device *dev,
	struct libusb_device_descriptor *desc)
{
	struct libusb_device_descriptor ld;

	if (!dev || !desc)
		return -EINVAL;

	ld = *((struct libusb_device_descriptor *)dev);
	memcpy(desc, &ld, sizeof(ld));

	return 0;
}
