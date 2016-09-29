/**
 * @mainpage Control USB power on a port by port basis on some USB hubs.
 *
 * This only works on USB hubs that have the hardware necessary to allow
 * software controlled power switching. Most hubs DO NOT include the
 * hardware.
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
#include <stdio.h>
#include <string.h>

#define USB_RT_HUB			(USB_TYPE_CLASS | USB_RECIP_DEVICE)
#define USB_RT_PORT			(USB_TYPE_CLASS | USB_RECIP_OTHER)
#define USB_PORT_FEAT_POWER		8
#define USB_PORT_FEAT_INDICATOR		22
#define USB_DIR_IN			0x80		/* to host */

#define COMMAND_SET_NONE		0
#define COMMAND_SET_LED			1
#define COMMAND_SET_POWER		2
#define HUB_LED_GREEN			2

#define HUB_CHAR_LPSM			0x0003
#define HUB_CHAR_PORTIND		0x0080

#define CTRL_TIMEOUT 			1000
#define USB_STATUS_SIZE 		4

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
		"Options:\n"
		"-h                     help\n"
		"-b     <bus-number>    USB bus number\n"
		"-d     <dev-number>    USB device number\n"
		"-n     <hub-ID>        USB hub ID\n"
		"-P     <port-ID>       ID of USB hub port\n"
		"-p     <enable>        Value enable or disable port [0, 1]\n"
		"-l     <leds>          Set USB hub LEDs to specified value[0, 1, 2, 3]\n"
		"-v                     verbose\n",
		progname);
}

static void exit_with_usage(const char *progname)
{
	usage(progname);
	exit(1);
}

static void hub_port_status(usb_dev_handle *uh, int nport)
{
	int i;

	printf(" Hub Port Status:\n");
	for (i = 0; i < nport; i++) {
		char buf[USB_STATUS_SIZE];
		int ret;

		ret = usb_control_msg (uh,
			USB_ENDPOINT_IN | USB_TYPE_CLASS | USB_RECIP_OTHER,
			USB_REQ_GET_STATUS, 0, i + 1, buf, USB_STATUS_SIZE,
			CTRL_TIMEOUT);
		if (ret < 0) {
			fprintf (stderr,
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
	struct usb_bus *busses;
	struct usb_bus *bus;

	num_hubs = 0;
	busses = usb_get_busses();
	if (busses == NULL) {
		perror("failed to access USB");
		return -1;
	}

	for (bus = busses; bus; bus = bus->next) {
		struct usb_device *dev;

		for (dev = bus->devices; dev; dev = dev->next) {
			usb_dev_handle *uh;
			int print = 0;

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

			char buf[1024];
			int len;
			int nport;
			struct usb_hub_descriptor *uhd = (struct usb_hub_descriptor *)buf;
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

int main(int argc, char *argv[])
{
	const char short_options[] = "hb:d:n:P:p:l:v";
	int busnum = 0;
	int devnum = 0;
	int cmd = COMMAND_SET_NONE;
	int port = 1;
	unsigned long argument = 0;
	int request, feature, index;
	int result = 0;
	int listing = 0;
	int verbose = 0;
	int hub = -1;
	usb_dev_handle *uh = NULL;
	int option;

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
				exit_with_usage (argv[0]);
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
				exit_with_usage (argv[0]);
			errno = 0;
			argument = strtoul(optarg, NULL, 10);
			if (errno != 0 || argument == 0) {
				fprintf(stderr, "Command line argument for -d is out of range.\n\n");
				exit(1);
			}
			devnum = argument;
			break;

		case 'P':
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
				exit_with_usage (argv[0]);
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
			if (cmd != COMMAND_SET_NONE)
				exit_with_usage(argv[0]);
			errno = 0;
			argument = strtoul(optarg, NULL, 10);
			if (errno != 0 || argument > 1) {
				fprintf(stderr, "Command line argument for -p is out of range.\n\n");
				usage(argv[0]);
				exit(1);
			}
			cmd = COMMAND_SET_POWER;
			break;

		case 'v':
			verbose = 1;
			if (argc == 2)
				listing = 1;
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

	if (cmd == COMMAND_SET_POWER) {
		if (argument)
			request = USB_REQ_SET_FEATURE;
		else
			request = USB_REQ_CLEAR_FEATURE;
		feature = USB_PORT_FEAT_POWER;
		index = port;
	} else {
		request = USB_REQ_SET_FEATURE;
		feature = USB_PORT_FEAT_INDICATOR;
		index = (argument << 8) | port;
		printf("port %02x value = %02lx\n", port, argument);
	}

	if (verbose) {
		printf("Send control message (REQUEST=%d, FEATURE=%d, INDEX=%04x)\n",
			request, feature, index);
	}

	if (usb_control_msg(uh, USB_RT_PORT, request, feature, index,
			NULL, 0, CTRL_TIMEOUT) < 0) {
		perror("failed to control.\n");
		result = 1;
	}

	if (verbose)
		hub_port_status(uh, hubs[hub].nport);

	usb_close(uh);

	exit(result);
}
