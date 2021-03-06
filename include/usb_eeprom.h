/**
 * @file
 * @author Meike Vocke <meike.vocke\@avionic-design.de>
 * @date 2016
 *
 * @brief USB hub EEPROM implementation for Cypress CY7C65620/CY7C65630
 *
 * @copyright GPLv3
 */

#ifndef USB_EEPROM_H
#define USB_EEPROM_H

#include <libusb.h>

/** USB control message Requesttype for EEPROM read */
#define USB_REQ_TYPE_READ_EEPROM	0xC0
/** USB control message Requesttype for EEPROM write */
#define USB_REQ_TYPE_WRITE_EEPROM	0x40
/** USB control message Request for read */
#define USB_REQ_READ			0x02
/** USB control message Request for write */
#define USB_REQ_WRITE			0x01
/** Time constant in ms for timeout per 256 Bytes for USB control message */
#define CTRL_TIMEOUT_PER_256_BYTES	1000
/** calculate the timeout for specific amount of bytes */
#define GET_TIMEOUT(t) (((t + 256 - 1) >> 8) * CTRL_TIMEOUT_PER_256_BYTES)
/** maximum EEPROM size */
#define MAX_EEPROM_SIZE 0x1000

/**
 * @defgroup eeprom_support_flags EEPROM support flags
 *
 * Flags indicating the level of support for EEPROM operations. Tells whether
 * an EEPROM exists, is blank or the USB device it is attached to even qualifies
 * for known EEPROM commands.
 *
 * @{
 */
#define EEPROM_SUPPORT_DEVICE		0x01	/**< USB device is qualified */
#define EEPROM_SUPPORT_STORAGE		0x02	/**< EEPROM is attached */
#define EEPROM_SUPPORT_BLANK		0x04	/**< Attached EEPROM is blank */
/** @} */

/**
 * @brief Erase EEPROM data
 *
 * Erase the data contained in the EEPROM of the USB hub.
 * This means putting 0xFF in every byte.
 *
 * @param dev pointer to the libusb_device_handle to use
 * @param size number of Bytes to erase
 * @return number of actually erased bytes on success
 * @return -errno on failure
 */
int usb_eeprom_erase(libusb_device_handle *dev, size_t size);

/**
 * @brief Read EEPROM data to buffer
 *
 * Read the data from the EEPROM of the USB hub.
 * The data will be written into the given buffer.
 *
 * @param dev pointer to the libusb_device_handle to use
 * @param buffer pointer where the data will be stored
 * @param size number of Bytes to read
 * @pre the buffer have to be allocated
 * @return number of actually read bytes on success
 * @return -errno on failure
 */
int usb_eeprom_read(libusb_device_handle *dev, uint8_t *buffer, size_t size);

/**
 * @brief Write buffer EEPROM data
 *
 * Write given data from pointer to EEPROM of the USB hub.
 *
 * @param dev pointer to the libusb_device_handle to use
 * @param buffer pointer where the data will be stored
 * @param size number of Bytes to write
 * @pre the buffer have to be allocated
 * @return number of actually written bytes on success
 * @return -errno on failure
 */
int usb_eeprom_write(libusb_device_handle *dev, uint8_t *buffer, size_t size);

/**
 * @brief Detect an attached EEPROM on a supported device
 *
 * @param dev pointer to a libusb_device
 * @return Combination of @ref eeprom_support_flags when the device is supprted
 * @return 0 for an unspported device
 * @return -EINVAL on failure if dev is @c NULL
 * @return -EIO if the descriptor cannot be read
 *
 * @note For now, only Cypress hubs (04b4:6560) are supported.
 */
int usb_eeprom_support(libusb_device *dev);

#endif /* USB_EEPROM_H */
