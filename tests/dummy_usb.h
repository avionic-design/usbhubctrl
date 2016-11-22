/**
 * @file
 * @author Meike Vocke <meike.vocke\@avionic-design.de>
 * @date 2016
 *
 * @brief libusb.h mocks for usb_eeprom functions testing
 *
 * @copyright GPLv3
 */

#ifndef DUMMY_USB_H
#define DUMMY_USB_H

#include <stdint.h>
#include <libusb.h>

/** struct for passed parameters of usb_control_msg */
struct usb_msg {
	/** usb requesttype */
	int requesttype;
	/** usb request */
	int request;
	/** pointer to a buffer */
	uint8_t *bytes;
	/** timeout in ms */
	int timeout;
	/** value for usb control message */
	int value;
	/** index für usb control message */
	int index;
	/** size of bytes in buffer */
	int size;
};

/** struct as for usb device */
struct libusb_device_handle {
	/** pointer to usb control message */
	struct usb_msg *msg;
};

/** typedef for use of libusb_device_handle as in libusb.h */
typedef struct libusb_device_handle libusb_device_handle;

/**
 * @brief Create a libusb_device_handle struct
 *
 * @return pointer to created libusb_device_handle on success
 * @return null pointer on failure
 */
libusb_device_handle *libusb_device_handle_create();

/**
 * @brief Free a libusb_device_handle struct
 *
 * @param dev pointer to libusb_device_handle
 * @return 0 on success
 * @return -errno on failure
 */
int libusb_device_handle_free(libusb_device_handle **dev);

/**
 * @brief Get usb control message from libusb_device_handle
 *
 * @param dev pointer to the usb device
 * @return usb_msg on success
 * @return null pointer when dev is not defined
 */
struct usb_msg *get_usb_msg(libusb_device_handle *dev);

#endif /* DUMMY_USB_H */
