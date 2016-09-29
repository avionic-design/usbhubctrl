/**
 * @file
 * @author Meike Vocke <meike.vocke\@avionic-design.de>
 * @date 2016
 *
 * @brief Provide usb.h functions and structs for usb_eeprom functions testing
 *
 * @copyright GPLv3
 */

#ifndef DUMMY_USB_H
#define DUMMY_USB_H

#include <stdint.h>

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
struct usb_dev_handle {
	/** pointer to usb control message */
	struct usb_msg *msg;
};

/** typedef for use of usb_dev_handle as in usb.h */
typedef struct usb_dev_handle usb_dev_handle;

/**
 * @brief Create a usb_dev_handle struct
 *
 * @return pointer to created usb_dev_handle on success
 * @return null pointer on failure
 */
usb_dev_handle *usb_dev_handle_create();

/**
 * @brief Free a usb_dev_handle struct
 *
 * @param dev pointer to usb_dev_handle
 * @return 0 on success
 * @return -errno on failure
 */
int usb_dev_handle_free(usb_dev_handle **dev);

/**
 * @brief Read data from file into a buffer
 *
 * This function sends a simple control message to a specified endpoint and
 * waits for the message to complete, or timeout.
 *
 * @param dev pointer to the usb device to send the message to
 * @param request USB message request value
 * @param requesttype USB message request type value
 * @param value USB message value
 * @param index USB message index value
 * @param bytes pointer to the data to send
 * @param size length in bytes of the data to send
 * @param timeout time in msecs to wait for the message to complete before
 * timing out (set to 0 to wait infinitely)
 * @return number of tranferred bytes on success
 * @return -errno on failure
 */
int usb_control_msg(usb_dev_handle *dev, int requesttype, int request,
		int value, int index, char *bytes, int size, int timeout);

/**
 * @brief Get usb control message from usb_device_handle
 *
 * @param dev pointer to the usb device
 * @return usb_msg on success
 * @return null pointer when dev is not defined
 */
struct usb_msg *get_usb_msg(usb_dev_handle *dev);

#endif /* DUMMY_USB_H */
