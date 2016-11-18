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
#include <getopt.h>
#include <usb.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "file_io.h"
#include "usb_eeprom.h"

#define USB_RT_HUB			(USB_TYPE_CLASS | USB_RECIP_DEVICE)
#define USB_RT_PORT			(USB_TYPE_CLASS | USB_RECIP_OTHER)
#define USB_PORT_FEAT_POWER		8
#define USB_PORT_FEAT_INDICATOR		22
#define USB_DIR_IN			0x80		/* to host */

#define COMMAND_SET_NONE		0
#define COMMAND_SET_LED			(1 << 0)
#define COMMAND_SET_POWER		(1 << 1)
#define COMMAND_GET_EEPROM		(1 << 2)
#define COMMAND_SET_EEPROM		(1 << 3)
#define COMMAND_CLR_EEPROM		(1 << 4)
#define HUB_LED_GREEN			2

#define HUB_CHAR_LPSM			0x0003
#define HUB_CHAR_PORTIND		0x0080

#define CTRL_TIMEOUT			1000
#define USB_STATUS_SIZE			4

#define MAX_HUBS 128

struct usb_hub_descriptor {
	unsigned char bDescLength;
	unsigned char bDescriptorType;
	unsigned char bNbrPorts;
	unsigned char wHubCharacteristics[2];
	unsigned char bPwrOn2PwrGood;
	unsigned char bHubContrCurrent;
	unsigned char data[0];
};

struct hub_info {
	int busnum;
	int devnum;
	struct usb_device *dev;
	int nport;
	int indicator_support;
};

static struct hub_info hubs[MAX_HUBS];
/** Number of hubs supporting power switching */
static int num_hubs;

static void usage(const char *progname)
{
	fprintf(stderr,
		"Usage: %s [{-n HUBNUM | -b BUSNUM -d DEVNUM}] [-v]\n"
		"          [-P PORT] [{-p [VALUE]|-l [VALUE]}]\n\n"
		"or:    %s [{-n HUBNUM | -b BUSNUM -d DEVNUM}] [-v]\n"
		"          [{-w BYTES -f filename} | {-r BYTES -f filename} | -e BYTES]\n\n"
		"Options:\n"
		"-b     <bus-number>    USB bus number\n"
		"-d     <dev-number>    USB device number\n"
		"-e     <N>             Erase N bytes in EEPROM\n"
		"-f     <filename>      filename, \"-\" for stdin/stdout, if not used a file \"output.iic\" was created\n"
		"-h                     help\n"
		"-l     <leds>          Set USB hub LEDs to specified value[0, 1, 2, 3]\n"
		"-n     <hub-ID>        USB hub ID\n"
		"-P     <port-ID>       ID of USB hub port\n"
		"-p     <enable>        Value enable or disable port [0, 1]\n"
		"-q     <quiet>         no output at all\n"
		"-r     <N>             Read N bytes from EEPROM\n"
		"-v                     verbose\n"
		"-w     <N>             Write N bytes to EEPROM\n",
		progname, progname);
}

static void exit_with_usage(const char *progname)
{
	usage(progname);
	exit(1);
}

static void hub_port_status(usb_dev_handle *uh, int nport)
{
	char buf[USB_STATUS_SIZE];
	int ret;
	int i;

	printf(" Hub Port Status:\n");
	for (i = 0; i < nport; i++) {

		ret = usb_control_msg(uh,
			USB_ENDPOINT_IN | USB_TYPE_CLASS | USB_RECIP_OTHER,
			USB_REQ_GET_STATUS, 0, i + 1, buf, USB_STATUS_SIZE,
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

static int usb_find_hubs(int listing, int verbose, int busnum, int devnum, int hub)
{
	struct usb_hub_descriptor *uhd = NULL;
	struct usb_bus *busses;
	struct usb_device *dev;
	struct usb_bus *bus;
	usb_dev_handle *uh;
	char buf[1024];
	int print = 0;
	int nport;
	int len;

	num_hubs = 0;
	busses = usb_get_busses();
	if (busses == NULL) {
		perror("failed to access USB");
		return -1;
	}

	for (bus = busses; bus; bus = bus->next) {

		for (dev = bus->devices; dev; dev = dev->next) {
			uhd = (struct usb_hub_descriptor *)buf;

			if (dev->descriptor.bDeviceClass != USB_CLASS_HUB &&
					dev->descriptor.bDeviceClass != USB_CLASS_VENDOR_SPEC)
				continue;

			if (listing || (verbose && ((atoi(bus->dirname) == busnum &&
					dev->devnum == devnum) ||
					hub == num_hubs))) {
				print = 1;
			}

			uh = usb_open(dev);

			if (uh == NULL)
				continue;

			len = usb_control_msg(uh,
				USB_DIR_IN | USB_RT_HUB, USB_REQ_GET_DESCRIPTOR,
				USB_DT_HUB << 8, 0, buf, sizeof(buf), CTRL_TIMEOUT);
			if (len <= sizeof(struct usb_hub_descriptor)) {
				perror("Can't get hub descriptor");
				usb_close(uh);
				continue;
			}

			if (!(uhd->wHubCharacteristics[0] & HUB_CHAR_PORTIND) &&
					(uhd->wHubCharacteristics[0] & HUB_CHAR_LPSM) >= 2)
				continue;

			if (print) {
				printf("Hub #%d at %s:%03d\n", num_hubs,
					bus->dirname, dev->devnum);

				switch ((uhd->wHubCharacteristics[0] & HUB_CHAR_LPSM)) {
				case 0:
					fprintf(stderr, " INFO: ganged switching.\n");
					break;
				case 1:
					fprintf(stderr, " INFO: individual power switching.\n");
					break;
				case 2:
				case 3:
					fprintf(stderr, " WARN: No power switching.\n");
					break;
				}

				if (!(uhd->wHubCharacteristics[0] & HUB_CHAR_PORTIND))
					fprintf(stderr, " WARN: Port indicators are NOT supported.\n");
			}

			nport = buf[2];
			hubs[num_hubs].busnum = atoi(bus->dirname);
			hubs[num_hubs].devnum = dev->devnum;
			hubs[num_hubs].dev = dev;
			hubs[num_hubs].indicator_support =
				(uhd->wHubCharacteristics[0] & HUB_CHAR_PORTIND) ? 1 : 0;
			hubs[num_hubs].nport = nport;

			num_hubs++;

			if (verbose)
				hub_port_status(uh, nport);

			usb_close(uh);
		}
	}

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

int main(int argc, char **argv)
{
	const char short_options[] = "b:d:e:f:hl:n:P:p:qr:vw:";
	int feature = USB_PORT_FEAT_INDICATOR;
	int request = USB_REQ_SET_FEATURE;
	char *default_file = "output.iic";
	int cmd = COMMAND_SET_NONE;
	unsigned long argument = 0;
	uint8_t *cmp_buffer = NULL;
	usb_dev_handle *uh = NULL;
	uint16_t erase_size = 0;
	uint16_t write_size = 0;
	uint16_t read_size = 0;
	uint8_t *buffer = NULL;
	char *filename = NULL;
	int listing = 0;
	int verbose = 0;
	int ret_val = 0;
	int busnum = 0;
	int devnum = 0;
	int result = 0;
	int index = 0;
	int quiet = 0;
	int port = 1;
	int hub = -1;
	int len = 0;
	int option;
	int i;

	if (argc == 1)
		listing = 1;

	for (;;) {
		option = getopt(argc, argv, short_options);
		if (option == -1)
			break;

		switch (option) {
		case 'h':
			usage(argv[0]);
			exit(0);
		case 'n':
			if (busnum > 0 || devnum > 0)
				exit_with_usage(argv[0]);
			errno = 0;
			argument = strtoul(optarg, NULL, 10);
			if (errno != 0 || argument == 0) {
				fprintf(stderr, "Command line argument for -n is out of range.\n\n");
				exit(1);
			}
			hub = argument;
			break;

		case 'b':
			if (hub >= 0)
				exit_with_usage(argv[0]);
			errno = 0;
			argument = strtoul(optarg, NULL, 10);
			if (errno != 0 || argument == 0) {
				fprintf(stderr, "Command line argument for -b is out of range.\n\n");
				exit(1);
			}
			busnum = argument;
			break;

		case 'd':
			if (hub >= 0)
				exit_with_usage(argv[0]);
			errno = 0;
			argument = strtoul(optarg, NULL, 10);
			if (errno != 0 || argument == 0) {
				fprintf(stderr, "Command line argument for -d is out of range.\n\n");
				exit(1);
			}
			devnum = argument;
			break;

		case 'P':
			if (cmd != COMMAND_SET_NONE && cmd != COMMAND_SET_POWER)
				exit_with_usage(argv[0]);
			cmd = COMMAND_SET_POWER;
			errno = 0;
			argument = strtoul(optarg, NULL, 10);
			if (errno != 0 || argument == 0) {
				fprintf(stderr, "Command line argument for -P is out of range.\n\n");
				exit(1);
			}
			port = argument;
			break;

		case 'l':
			if (cmd != COMMAND_SET_NONE)
				exit_with_usage(argv[0]);
			errno = 0;
			argument = strtoul(optarg, NULL, 10);
			if (errno != 0 || argument > 3) {
				fprintf(stderr, "Command line argument for -l is out of range.\n\n");
				usage(argv[0]);
				exit(1);
			}
			cmd = COMMAND_SET_LED;
			break;

		case 'p':
			if (cmd != COMMAND_SET_NONE && cmd != COMMAND_SET_POWER)
				exit_with_usage(argv[0]);
			cmd = COMMAND_SET_POWER;
			errno = 0;
			argument = strtoul(optarg, NULL, 10);
			if (errno != 0 || argument > 1) {
				fprintf(stderr, "Command line argument for -p is out of range.\n\n");
				usage(argv[0]);
				exit(1);
			}
			break;

		case 'v':
			verbose = 1;
			if (argc == 2)
				listing = 1;
			break;

		case 'q':
			quiet = 1;
			break;

		case 'r':
			if (cmd != COMMAND_SET_NONE)
				exit_with_usage(argv[0]);
			cmd = COMMAND_GET_EEPROM;
			errno = 0;
			argument = strtoul(optarg, NULL, 10);
			if (errno != 0) {
				fprintf(stderr, "Command line argument for -r is out of range.\n\n");
				usage(argv[0]);
				exit(1);
			} else if (argument > MAX_EEPROM_SIZE || !argument) {
				fprintf(stderr, "Command line argument for -r is invalid.\n\n");
				usage(argv[0]);
				exit(1);
			}
			read_size = argument;
			break;

		case 'w':
			if (cmd != COMMAND_SET_NONE)
				exit_with_usage(argv[0]);
			cmd = COMMAND_SET_EEPROM;
			errno = 0;
			argument = strtoul(optarg, NULL, 10);
			if (errno != 0) {
				fprintf(stderr, "Command line argument for -w is out of range.\n\n");
				usage(argv[0]);
				exit(1);
			} else if (argument > MAX_EEPROM_SIZE || !argument) {
				fprintf(stderr, "Command line argument for -w is invalid.\n\n");
				usage(argv[0]);
				exit(1);
			}
			write_size = argument;
			break;

		case 'e':
			if (cmd != COMMAND_SET_NONE)
				exit_with_usage(argv[0]);
			cmd = COMMAND_CLR_EEPROM;
			errno = 0;
			argument = strtoul(optarg, NULL, 10);
			if (errno != 0) {
				fprintf(stderr, "Command line argument for -e is out of range.\n\n");
				usage(argv[0]);
				exit(1);
			} else if (argument > MAX_EEPROM_SIZE) {
				fprintf(stderr, "Command line argument for -e is invalid.\n\n");
				usage(argv[0]);
				exit(1);
			}
			erase_size = argument;
			break;

		case 'f':
			filename = optarg;
			break;

		default:
			exit_with_usage(argv[0]);
		}
	}

	/* BUS is specified, but DEV isn't, or ... */
	if ((busnum > 0 && devnum <= 0) || (busnum <= 0 && devnum > 0))
		exit_with_usage(argv[0]);

	/* Default is the hub #0 */
	if (hub < 0 && busnum == 0)
		hub = 0;

	/* Default is POWER */
	if (cmd == COMMAND_SET_NONE)
		cmd = COMMAND_SET_POWER;

	usb_init();
	usb_find_busses();
	usb_find_devices();

	if (usb_find_hubs(listing, verbose, busnum, devnum, hub) <= 0) {
		fprintf(stderr, "No hubs found.\n");
		exit(1);
	}

	if (listing)
		exit(0);

	if (hub < 0)
		hub = get_hub(busnum, devnum);

	if (hub >= 0 && hub < num_hubs)
		uh = usb_open(hubs[hub].dev);

	if (uh == NULL) {
		fprintf(stderr, "Device not found.\n");
		exit(1);
	}

	switch (cmd) {
	case COMMAND_GET_EEPROM:
		buffer = malloc(read_size);
		if (!buffer) {
			fprintf(stderr, "Malloc failed with error %s\n",
					strerror(errno));
			result = 1;
			goto cleanup;
		}

		ret_val = usb_eeprom_read(uh, buffer, read_size);
		if (ret_val != read_size) {
			fprintf(stderr, "EEPROM read failed.\n");
			result = 1;
			goto cleanup;
		}

		if (verbose) {
			for (i = 0; i < ret_val; i++) {
				if (!(i % 16))
					printf("\n %04x:   ", i);
				printf("%02X ", buffer[i]);
			}
			putchar('\n');
		}

		read_size = ret_val;
		if (!filename)
			filename = default_file;

		ret_val = file_write(filename, buffer, read_size);
		if (ret_val != read_size) {
			fprintf(stderr, "Write file failed.\n");
			result = 1;
			goto cleanup;
		}

		if (strcmp(filename, "-") != 0 && verbose)
			printf("EEPROM written to %s\n", filename);
		break;
	case COMMAND_SET_EEPROM:
		if (!filename) {
			fprintf(stderr, "No file name specified.\n");
			result = 1;
			goto cleanup;
		}
		ret_val = file_read(filename, &buffer, write_size);
		if (ret_val < 0) {
			fprintf(stderr, "Read file failed.\n");
			result = 1;
			goto cleanup;
		}

		/* switch write size to actually read number of bytes from file */
		write_size = ret_val;

		ret_val = usb_eeprom_write(uh, buffer, write_size);
		if (ret_val != write_size) {
			fprintf(stderr, "EEPROM write failed.\n");
			result = 1;
			goto cleanup;
		}

		cmp_buffer = malloc(write_size);
		if (!cmp_buffer) {
			fprintf(stderr, "Malloc failed with error %s\n",
					strerror(errno));
			result = 1;
			goto cleanup;
		}
		ret_val = usb_eeprom_read(uh, cmp_buffer, write_size);
		if (ret_val != write_size) {
			fprintf(stderr, "EEPROM read failed.\n");
			result = 1;
			goto cleanup;
		}

		if (memcmp(buffer, cmp_buffer, write_size) != 0) {
			fprintf(stderr, "EEPROM writing failed\n");
			result = 1;
			goto cleanup;
		} else if (!quiet) {
			printf("File content successfully written %i bytes to EEPROM\n",
					write_size);
		}

		break;
	case COMMAND_CLR_EEPROM:
		ret_val = usb_eeprom_erase(uh, erase_size);

		if (ret_val == erase_size)
			break;

		if (ret_val < 0) {
			fprintf(stderr, "EEPROM erase failed with error code: %s\n",
					usb_strerror());
		} else {
			fprintf(stderr, "EEPROM erase failed, %i bytes erased instead of %i bytes\n",
					ret_val, erase_size);
		}

		result = 1;
		goto cleanup;
	case COMMAND_SET_POWER:
		if (argument)
			request = USB_REQ_SET_FEATURE;
		else
			request = USB_REQ_CLEAR_FEATURE;
		feature = USB_PORT_FEAT_POWER;
		index = port;
		len = usb_control_msg(uh, USB_RT_PORT, request, feature, index,
				NULL, 0, CTRL_TIMEOUT);
		if (len < 0) {
			fprintf(stderr, "usb_control_msg failed, error code: %s.\n",
					usb_strerror());
			result = 1;
			goto cleanup;
		}
		break;
	default:
		request = USB_REQ_SET_FEATURE;
		feature = USB_PORT_FEAT_INDICATOR;
		index = (argument << 8) | port;
		if (!quiet)
			printf("port %02x value = %02lx\n", port, argument);
		len = usb_control_msg(uh, USB_RT_PORT, request, feature, index,
				NULL, 0, CTRL_TIMEOUT);
		if (len < 0) {
			fprintf(stderr, "usb_control_msg failed, error code: %s.\n",
					usb_strerror());
			result = 1;
			goto cleanup;
		}
	}

	if (verbose && !(cmd & (COMMAND_CLR_EEPROM | COMMAND_GET_EEPROM | COMMAND_SET_EEPROM))) {
		printf("Sent control message (REQUEST=%d, FEATURE=%d, INDEX=%04x)\n",
			request, feature, index);
	}

cleanup:

	if (verbose && !(cmd & (COMMAND_CLR_EEPROM | COMMAND_GET_EEPROM | COMMAND_SET_EEPROM)))
		hub_port_status(uh, hubs[hub].nport);

	usb_close(uh);

	if (buffer)
		free(buffer);

	if (cmp_buffer)
		free(cmp_buffer);

	exit(result);
}
