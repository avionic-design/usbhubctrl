/**
 * @file
 * @author Bert van Hall <bert.vanhall\@avionic-design.de>
 * @date 2016
 *
 * @brief Testing data for the usb_eeprom testsuite
 *
 * @copyright GPLv3
 */

#ifndef CHECK_USB_EEPROM_DATA_H
#define CHECK_USB_EEPROM_DATA_H

#include <libusb.h>
#include "usb_eeprom.h"

/** Structure for test_eeprom_support() testing data */
struct data_eesupport {
	struct libusb_device_descriptor desc;	/**< USB descriptor (input) */
	int ret;				/**< Return code (output) */
};

/** Testing data for test_eeprom_support() */
struct data_eesupport eeprom_support_data[] = {
	{
		.desc = {
			.idVendor = 0, .idProduct = 0,
			.bDeviceClass = 0, .bcdDevice = 0
		},
		.ret = 0
	},
	{
		.desc = {
			.idVendor = 0x04b4, .idProduct = 0,
			.bDeviceClass = 0, .bcdDevice = 0
		},
		.ret = 0
	},
	{
		.desc = {
			.idVendor = 0, .idProduct = 0x6560,
			.bDeviceClass = 0, .bcdDevice = 0
		},
		.ret = 0
	},
	{
		.desc = {
			.idVendor = 0, .idProduct = 0,
			.bDeviceClass = LIBUSB_CLASS_HUB, .bcdDevice = 0x9415
		},
		.ret = 0
	},
	{
		.desc = {
			.idVendor = 0xadad, .idProduct = 0x0001,
			.bDeviceClass = LIBUSB_CLASS_HUB, .bcdDevice = 0x9415
		},
		.ret = 0
	},
	{
		.desc = {
			.idVendor = 0xadad, .idProduct = 0x0042,
			.bDeviceClass = LIBUSB_CLASS_VENDOR_SPEC, .bcdDevice = 0x9015
		},
		.ret = 0
	},
	{
		.desc = {
			.idVendor = 0x04b4, .idProduct = 0x6560,
			.bDeviceClass = 0, .bcdDevice = 0
		},
		.ret = EEPROM_SUPPORT_DEVICE
	},
	{
		.desc = {
			.idVendor = 0x04b4, .idProduct = 0x6560,
			.bDeviceClass = LIBUSB_CLASS_HUB, .bcdDevice = 0
		},
		.ret = EEPROM_SUPPORT_DEVICE | EEPROM_SUPPORT_STORAGE
	},
	{
		.desc = {
			.idVendor = 0x04b4, .idProduct = 0x6560,
			.bDeviceClass = LIBUSB_CLASS_HUB, .bcdDevice = 0x9215
		},
		.ret = EEPROM_SUPPORT_DEVICE | EEPROM_SUPPORT_STORAGE
	},
	{
		.desc = {
			.idVendor = 0x04b4, .idProduct = 0x6560,
			.bDeviceClass = LIBUSB_CLASS_HUB, .bcdDevice = 0x9415
		},
		.ret = EEPROM_SUPPORT_DEVICE | EEPROM_SUPPORT_STORAGE
	},
	{
		.desc = {
			.idVendor = 0x04b4, .idProduct = 0x6560,
			.bDeviceClass = LIBUSB_CLASS_VENDOR_SPEC, .bcdDevice = 0x9015
		},
		.ret = EEPROM_SUPPORT_DEVICE | EEPROM_SUPPORT_STORAGE | EEPROM_SUPPORT_BLANK
	},
	{
		.desc = {
			.idVendor = 0x04b4, .idProduct = 0x6560,
			.bDeviceClass = LIBUSB_CLASS_VENDOR_SPEC, .bcdDevice = 0x9215
		},
		.ret = EEPROM_SUPPORT_DEVICE
	},
	{
		.desc = {
			.idVendor = 0x04b4, .idProduct = 0x6560,
			.bDeviceClass = LIBUSB_CLASS_HUB, .bcdDevice = 0x9015
		},
		.ret = EEPROM_SUPPORT_DEVICE
	}
};

#endif /* CHECK_USB_EEPROM_DATA_H */
