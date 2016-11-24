/**
 * @mainpage Control USB hub.
 *
 * For each USB hub port the power can be switched on or off and the status
 * indicator LED can be set to either automatic mode or one of green, amber, and
 * off.
 *
 * This only works on USB hubs that have the hardware necessary to allow
 * software controlled power switching. Most hubs DO NOT include the
 * hardware.
 *
 * Furthermore a interface to the connected EEPROM is implemented. Thus the
 * EEPROM can be written and read through hub-ctrl. A functionality to erase the
 * EEPROM is also implemented (Writing 0xFF to the EEPROM bytes). This Interface
 * is developed for Cypress USB Hubs (CY7C65620/CY7C65630).
 *
 * @author NIIBE Yutaka <gniibe at fsij.org>
 * @author Bert van Hall <bert.vanhall\@avionic-design.de>
 * @author Meike Vocke <meike.vocke\@avionic-design.de>
 * @date 2006-2016
 * @copyright GPLv3
 */

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libusb.h>

#include "file_io.h"
#include "options.h"
#include "usb_eeprom.h"

#define USB_RT_HUB			(LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_DEVICE)
#define USB_RT_PORT			(LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_OTHER)
#define USB_PORT_FEAT_POWER		8
#define USB_PORT_FEAT_INDICATOR		22

#define HUB_LED_GREEN			2

#define HUB_CHAR_LPSM			0x0003
#define HUB_CHAR_PORTIND		0x0080

#define CTRL_TIMEOUT			1000
#define USB_STATUS_SIZE			4

#define MAX_HUBS 128

struct usb_hub_descriptor {
	uint8_t bDescLength;
	uint8_t bDescriptorType;
	uint8_t bNbrPorts;
	uint16_t wHubCharacteristics;
	uint8_t bPwrOn2PwrGood;
	uint8_t bHubContrCurrent;
} __attribute__((packed));

struct hub_info {
	int busnum;
	int devnum;
	libusb_device *dev;
	int nport;
	int indicator_support;
};

static struct hub_info hubs[MAX_HUBS];
/** Number of hubs supporting power switching */
static int num_hubs;

static void hub_port_status(libusb_device_handle *dev, int nport)
{
	uint8_t buf[USB_STATUS_SIZE];
	int ret;
	int i;

	if (!dev)
		return;

	printf(" Hub Port Status:\n");
	for (i = 0; i < nport; i++) {

		ret = libusb_control_transfer(dev,
			LIBUSB_ENDPOINT_IN | USB_RT_PORT,
			LIBUSB_REQUEST_GET_STATUS, 0, i + 1, buf, USB_STATUS_SIZE,
			CTRL_TIMEOUT);
		if (ret < 0) {
			fprintf(stderr,
				"cannot read port %d status, %s (%d)\n",
				i + 1, strerror(errno), errno);
			break;
		}

		printf("   Port %d: %02x%02x.%02x%02x", i + 1, buf[3], buf[2],
			buf[1], buf[0]);

		printf("%s%s%s%s%s",
			(buf[2] & 0x10) ? " C_RESET" : "",
			(buf[2] & 0x08) ? " C_OC" : "",
			(buf[2] & 0x04) ? " C_SUSPEND" : "",
			(buf[2] & 0x02) ? " C_ENABLE" : "",
			(buf[2] & 0x01) ? " C_CONNECT" : "");

		printf("%s%s%s%s%s%s%s%s%s%s\n",
			(buf[1] & 0x10) ? " indicator" : "",
			(buf[1] & 0x08) ? " test" : "",
			(buf[1] & 0x04) ? " highspeed" : "",
			(buf[1] & 0x02) ? " lowspeed" : "",
			(buf[1] & 0x01) ? " power" : "",
			(buf[0] & 0x10) ? " RESET" : "",
			(buf[0] & 0x08) ? " oc" : "",
			(buf[0] & 0x04) ? " suspend" : "",
			(buf[0] & 0x02) ? " enable" : "",
			(buf[0] & 0x01) ? " connect" : "");
	}
}

static int usb_find_hubs(int print)
{
	struct libusb_device_descriptor desc;
	struct usb_hub_descriptor hub_desc;
	libusb_device_handle *dev = NULL;
	libusb_device **devlist;
	libusb_device *hub;
	uint8_t buf[sizeof(hub_desc)];
	uint8_t id_node;
	uint8_t id_bus;
	int ret;
	int len;
	int num;
	int i;

	num_hubs = 0;

	num = libusb_get_device_list(NULL, &devlist);
	if (num < 0) {
		fprintf(stderr, "Failed to get USB device list: %s\n",
			libusb_strerror(num));
		return -ENODEV;
	}

	if (print)
		printf("%d USB devices found.\n", num);

	for (i = 0; i < num; i++) {
		if (dev) {
			libusb_close(dev);
			dev = NULL;
		}

		hub = devlist[i];
		id_bus = libusb_get_bus_number(hub);
		id_node = libusb_get_device_address(hub);
		memset(&desc, 0, sizeof(desc));

		ret = libusb_get_device_descriptor(hub, &desc);
		if (ret && print > 1) {
			fprintf(stderr, "Device %03d:%03d: No descriptor: %s\n",
				id_bus, id_node, libusb_strerror(ret));
		}

		ret = libusb_open(hub, &dev);
		if (ret) {
			if (print > 1) {
				fprintf(stderr, "Device %03d:%03d (%04x:%04x): "
					"Failed to open: %s\n",
					id_bus, id_node, desc.idVendor,
					desc.idProduct, libusb_strerror(ret));
			}
			continue;
		}

		len = libusb_control_transfer(dev,
			LIBUSB_ENDPOINT_IN | USB_RT_HUB,
			LIBUSB_REQUEST_GET_DESCRIPTOR,
			LIBUSB_DT_HUB << 8, 0, buf, sizeof(buf), CTRL_TIMEOUT);

		if (len <= 0) {
			if (print > 1) {
				fprintf(stderr, "Device %03d:%03d (%04x:%04x): "
					"Failed to get descriptor: %s\n",
					id_bus, id_node, desc.idVendor,
					desc.idProduct, len < 0 ?
						libusb_strerror(len) :
						"None found.");
			}
			continue;
		}

		memset(&hub_desc, 0, sizeof(hub_desc));
		memcpy(&hub_desc, buf, len);

		if (!(hub_desc.wHubCharacteristics & HUB_CHAR_PORTIND) &&
				(hub_desc.wHubCharacteristics & HUB_CHAR_LPSM) >= 2) {
			if (print > 1) {
				fprintf(stderr, "Device %03d:%03d (%04x:%04x): "
					"Neither power switching nor "
					"indicators supported.\n",
					id_bus, id_node, desc.idVendor,
					desc.idProduct);
			}
			continue;
		}

		if (print) {
			printf("Device %03d:%03d (%04x:%04x): Supported!\n",
				id_bus, id_node, desc.idVendor, desc.idProduct);
		}

		if (print) {
			switch ((hub_desc.wHubCharacteristics & HUB_CHAR_LPSM)) {
			case 0:
				fprintf(stderr, "  INFO: ganged switching.\n");
				break;
			case 1:
				fprintf(stderr, "  INFO: individual power switching.\n");
				break;
			case 2:
			case 3:
				fprintf(stderr, "  WARN: No power switching.\n");
				break;
			}

			if (!(hub_desc.wHubCharacteristics & HUB_CHAR_PORTIND))
				fprintf(stderr, "  WARN: Port indicators are NOT supported.\n");
		}

		hubs[num_hubs].busnum = id_bus;
		hubs[num_hubs].devnum = id_node;
		hubs[num_hubs].dev = libusb_ref_device(hub);
		hubs[num_hubs].indicator_support = (buf[4] & HUB_CHAR_PORTIND) ? 1 : 0;
		hubs[num_hubs].nport = buf[2];
		num_hubs++;

		if (print)
			hub_port_status(dev, buf[2]);

		libusb_close(dev);
		dev = NULL;
	}

	libusb_free_device_list(devlist, 1);

	if (print)
		printf("%d supported hubs found.\n", num_hubs);

	return num_hubs;
}

int get_hub(int busnum, int devnum)
{
	int i;

	for (i = 0; i < num_hubs; i++)
		if (hubs[i].busnum == busnum && hubs[i].devnum == devnum)
			return i;

	return -1;
}

void clean_hub_info(struct hub_info *hubs, int len)
{
	int i;

	for (i = 0; i < len; i++)
		libusb_unref_device(hubs[i].dev);
}

int main(int argc, char **argv)
{
	int feature = USB_PORT_FEAT_INDICATOR;
	int request = LIBUSB_REQUEST_SET_FEATURE;
	char *default_file = "output.iic";
	libusb_device_handle *dev = NULL;
	struct hub_options opts = {
		.cmd = COMMAND_SET_NONE,
		.filename = NULL,
		.eesize = 0,
		.busnum = 0,
		.devnum = 0,
		.power = 0,
		.port = 1,
		.verbose = 0,
		.listing = 0,
		.quiet = 0,
	};
	uint8_t *cmp_buffer = NULL;
	uint8_t *buffer = NULL;
	int ret_val = 0;
	int result = 0;
	int index = 0;
	int len = 0;
	int hub = 0;
	int i;

	ret_val = options_scan(&opts, argc, argv);
	if (ret_val <= 0) {
		options_help(argv[0]);
		exit(ret_val);
	}

	/* BUS is specified, but DEV isn't, or ... */
	if ((opts.busnum > 0 && opts.devnum <= 0) ||
			(opts.busnum <= 0 && opts.devnum > 0)) {
		options_help(argv[0]);
		exit(ret_val);
	}

	/* Default is POWER */
	if (opts.cmd == COMMAND_SET_NONE)
		opts.cmd = COMMAND_SET_POWER;

	libusb_init(NULL);

	if (usb_find_hubs(opts.listing * (1 + opts.verbose)) <= 0) {
		fprintf(stderr, "No hubs found.\n");
		result = 1;
		goto cleanup;
	}

	if (opts.listing) {
		result = 0;
		goto cleanup;
	}

	hub = get_hub(opts.busnum, opts.devnum);
	if (hub >= 0) {
		ret_val = libusb_open(hubs[hub].dev, &dev);
		if (ret_val) {
			fprintf(stderr, "Failed to open device: %s\n",
				libusb_strerror(ret_val));
		}
	}

	if (dev == NULL) {
		fprintf(stderr, "Device not found.\n");
		result = 1;
		goto cleanup;
	}

	switch (opts.cmd) {
	case COMMAND_GET_EEPROM:
		buffer = malloc(opts.eesize);
		if (!buffer) {
			fprintf(stderr, "Malloc failed with error %s\n",
					strerror(errno));
			result = 1;
			goto cleanup;
		}

		ret_val = usb_eeprom_read(dev, buffer, opts.eesize);
		if (ret_val != opts.eesize) {
			fprintf(stderr, "EEPROM read failed: %d\n", ret_val);
			result = 1;
			goto cleanup;
		}

		if (opts.verbose) {
			for (i = 0; i < ret_val; i++) {
				if (!(i % 16))
					printf("\n %04x:   ", i);
				printf("%02X ", buffer[i]);
			}
			putchar('\n');
		}

		if (!opts.filename)
			opts.filename = default_file;

		ret_val = file_write(opts.filename, buffer, opts.eesize);
		if (ret_val != opts.eesize) {
			fprintf(stderr, "Write file failed.\n");
			result = 1;
			goto cleanup;
		}

		if (strcmp(opts.filename, "-") != 0 && opts.verbose)
			printf("EEPROM written to %s\n", opts.filename);
		break;
	case COMMAND_SET_EEPROM:
		if (!opts.filename) {
			fprintf(stderr, "No file name specified.\n");
			result = 1;
			goto cleanup;
		}
		ret_val = file_read(opts.filename, &buffer, opts.eesize);
		if (ret_val < 0) {
			fprintf(stderr, "Read file failed.\n");
			result = 1;
			goto cleanup;
		}

		/* switch write size to actually read number of bytes from file */
		len = ret_val;

		ret_val = usb_eeprom_write(dev, buffer, len);
		if (ret_val != len) {
			fprintf(stderr, "EEPROM write failed.\n");
			result = 1;
			goto cleanup;
		}

		cmp_buffer = malloc(len);
		if (!cmp_buffer) {
			fprintf(stderr, "Malloc failed with error %s\n",
					strerror(errno));
			result = 1;
			goto cleanup;
		}
		ret_val = usb_eeprom_read(dev, cmp_buffer, len);
		if (ret_val != len) {
			fprintf(stderr, "EEPROM read failed.\n");
			result = 1;
			goto cleanup;
		}

		if (memcmp(buffer, cmp_buffer, len) != 0) {
			fprintf(stderr, "EEPROM writing failed\n");
			result = 1;
			goto cleanup;
		} else if (!opts.quiet) {
			printf("File content successfully written %i bytes to EEPROM\n",
					len);
		}

		break;
	case COMMAND_CLR_EEPROM:
		ret_val = usb_eeprom_erase(dev, opts.eesize);

		if (ret_val == opts.eesize)
			break;

		if (ret_val < 0) {
			fprintf(stderr, "EEPROM erase failed with error code: %s\n",
					libusb_strerror(ret_val));
		} else {
			fprintf(stderr, "EEPROM erase failed, %i bytes erased instead of %i bytes\n",
					ret_val, opts.eesize);
		}

		result = 1;
		goto cleanup;
	case COMMAND_SET_POWER:
		if (opts.power)
			request = LIBUSB_REQUEST_SET_FEATURE;
		else
			request = LIBUSB_REQUEST_CLEAR_FEATURE;
		feature = USB_PORT_FEAT_POWER;
		index = opts.port;
		len = libusb_control_transfer(dev, USB_RT_PORT, request, feature, index,
				NULL, 0, CTRL_TIMEOUT);
		if (len < 0) {
			fprintf(stderr, "libusb_control_transfer failed, error code: %s.\n",
					libusb_strerror(len));
			result = 1;
			goto cleanup;
		}
		break;
	default:
		request = LIBUSB_REQUEST_SET_FEATURE;
		feature = USB_PORT_FEAT_INDICATOR;
		index = (opts.power << 8) | opts.port;
		if (!opts.quiet)
			printf("port %02x value = %02x\n", opts.port, opts.power);
		len = libusb_control_transfer(dev, USB_RT_PORT, request, feature, index,
				NULL, 0, CTRL_TIMEOUT);
		if (len < 0) {
			fprintf(stderr, "libusb_control_transfer failed, error code: %s.\n",
					libusb_strerror(len));
			result = 1;
			goto cleanup;
		}
	}

	if (opts.verbose && !(opts.cmd & COMMAND_TYPE_EEPROM)) {
		printf("Sent control message (REQUEST=%d, FEATURE=%d, INDEX=%04x)\n",
			request, feature, index);
	}

cleanup:
	if (opts.verbose && dev && !(opts.cmd & COMMAND_TYPE_EEPROM))
		hub_port_status(dev, hubs[hub].nport);

	if (dev)
		libusb_close(dev);

	clean_hub_info(hubs, num_hubs);

	libusb_exit(NULL);

	if (buffer)
		free(buffer);

	if (cmp_buffer)
		free(cmp_buffer);

	exit(result);
}
