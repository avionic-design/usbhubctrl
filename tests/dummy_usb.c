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

usb_dev_handle *usb_dev_handle_create()
{
	usb_dev_handle *uh = NULL;

	uh = malloc(sizeof(usb_dev_handle));
	if (!uh)
		return NULL;

	uh->msg = calloc(1, sizeof(struct usb_msg));
	if (!(uh->msg)) {
		free(uh);
		return NULL;
	}

	return uh;
}

int usb_dev_handle_free(usb_dev_handle **dev)
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

int usb_control_msg(usb_dev_handle *dev, int requesttype, int request,
		int value, int index, char *bytes, int size, int timeout)
{
	if (!dev || !dev->msg || ! bytes || !size)
		return -EINVAL;

	if (requesttype != USB_REQ_TYPE_READ_EEPROM &&
			requesttype != USB_REQ_TYPE_WRITE_EEPROM)
		return -ERANGE;

	if (request != USB_REQ_READ && request != USB_REQ_WRITE)
		return -ERANGE;

	dev->msg->bytes = malloc(size);
	if (!dev->msg->bytes)
		return -ENOMEM;

	dev->msg->requesttype = requesttype;
	dev->msg->request = request;
	dev->msg->value = value;
	dev->msg->index = index;
	memcpy(dev->msg->bytes, bytes, size);
	dev->msg->size = size;
	dev->msg->timeout = timeout;

	return size;
}

struct usb_msg *get_usb_msg(usb_dev_handle *dev)
{
	if (!dev)
		return NULL;

	return dev->msg;
}

