/**
 * @file
 * @author Bert van Hall <bert.vanhall\@avionic-design.de>
 * @date 2016
 *
 * @brief Options parsing for hub-ctrl
 *
 * @copyright GPLv3
 */

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "options.h"

#define EEPROM_SIZE_LIMIT	4096

void options_help(const char *progname)
{
	fprintf(stderr,
		"Usage: %s [{-b BUSNUM -d DEVNUM}] [-v] [-l]\n"
		"          [-P PORT] [{-p [VALUE]|-i [VALUE]}]\n\n"
		"or:    %s [{-b BUSNUM -d DEVNUM}] [-v]\n"
		"          [{-w BYTES -f filename} | {-r BYTES -f filename} | -e BYTES]\n\n"
		"Options:\n"
		"-b     <bus-number>    USB bus number\n"
		"-d     <dev-number>    USB device number\n"
		"-e     <N>             Erase N bytes in EEPROM\n"
		"-f     <filename>      filename, \"-\" for stdin/stdout, if not used a file \"output.iic\" was created\n"
		"-h                     help\n"
		"-i     <indicator>     Set USB hub indicators to specified value[0, 1, 2, 3]\n"
		"-l                     Scan for and list supported hubs\n"
		"-P     <port-ID>       ID of USB hub port\n"
		"-p     <enable>        Value enable or disable port [0, 1]\n"
		"-q     <quiet>         no output at all\n"
		"-r     <N>             Read N bytes from EEPROM\n"
		"-v                     verbose\n"
		"-w     <N>             Write N bytes to EEPROM\n",
		progname, progname);
}

int options_scan(struct hub_options *hargs, int argc, char **argv)
{
	const char short_options[] = "b:d:e:f:hi:lP:p:qr:vw:";
	unsigned long argument = 0;
	int option;

	if (!hargs)
		return -EINVAL;

	for (;;) {
		option = getopt(argc, argv, short_options);
		if (option == -1)
			break;

		switch (option) {
		case 'h':
			return 0;

		case 'l':
			hargs->listing = 1;
			break;

		case 'b':
			errno = 0;
			argument = strtoul(optarg, NULL, 10);
			if (errno != 0 || argument == 0) {
				fprintf(stderr, "Command line argument for -b is out of range.\n\n");
				return -ERANGE;
			}
			hargs->busnum = argument;
			break;

		case 'd':
			errno = 0;
			argument = strtoul(optarg, NULL, 10);
			if (errno != 0 || argument == 0) {
				fprintf(stderr, "Command line argument for -d is out of range.\n\n");
				return -ERANGE;
			}
			hargs->devnum = argument;
			break;

		case 'P':
			if (hargs->cmd != COMMAND_SET_NONE && hargs->cmd != COMMAND_SET_POWER)
				return -EINVAL;
			hargs->cmd = COMMAND_SET_POWER;
			errno = 0;
			argument = strtoul(optarg, NULL, 10);
			if (errno != 0 || argument == 0) {
				fprintf(stderr, "Command line argument for -P is out of range.\n\n");
				return -ERANGE;
			}
			hargs->port = argument;
			break;

		case 'i':
			if (hargs->cmd != COMMAND_SET_NONE)
				return -EINVAL;
			errno = 0;
			argument = strtoul(optarg, NULL, 10);
			if (errno != 0 || argument > 3) {
				fprintf(stderr, "Command line argument for -i is out of range.\n\n");
				return -ERANGE;
			}
			hargs->cmd = COMMAND_SET_LED;
			hargs->power = argument;
			break;

		case 'p':
			if (hargs->cmd != COMMAND_SET_NONE && hargs->cmd != COMMAND_SET_POWER)
				return -EINVAL;
			hargs->cmd = COMMAND_SET_POWER;
			errno = 0;
			argument = strtoul(optarg, NULL, 10);
			if (errno != 0 || argument > 1) {
				fprintf(stderr, "Command line argument for -p is out of range.\n\n");
				return -ERANGE;
			}
			hargs->power = argument;
			break;

		case 'v':
			hargs->verbose = 1;
			break;

		case 'q':
			hargs->quiet = 1;
			break;

		case 'r':
			if (hargs->cmd != COMMAND_SET_NONE)
				return -EINVAL;
			hargs->cmd = COMMAND_GET_EEPROM;
			errno = 0;
			argument = strtoul(optarg, NULL, 10);
			if (errno != 0) {
				fprintf(stderr, "Command line argument for -r is out of range.\n\n");
				return -ERANGE;
			} else if (argument > EEPROM_SIZE_LIMIT || !argument) {
				fprintf(stderr, "Command line argument for -r is invalid.\n\n");
				return -EINVAL;
			}
			hargs->eesize = argument;
			break;

		case 'w':
			if (hargs->cmd != COMMAND_SET_NONE)
				return -EINVAL;
			hargs->cmd = COMMAND_SET_EEPROM;
			errno = 0;
			argument = strtoul(optarg, NULL, 10);
			if (errno != 0) {
				fprintf(stderr, "Command line argument for -w is out of range.\n\n");
				return -ERANGE;
			} else if (argument > EEPROM_SIZE_LIMIT || !argument) {
				fprintf(stderr, "Command line argument for -w is invalid.\n\n");
				return -EINVAL;
			}
			hargs->eesize = argument;
			break;

		case 'e':
			if (hargs->cmd != COMMAND_SET_NONE)
				return -EINVAL;
			hargs->cmd = COMMAND_CLR_EEPROM;
			errno = 0;
			argument = strtoul(optarg, NULL, 10);
			if (errno != 0) {
				fprintf(stderr, "Command line argument for -e is out of range.\n\n");
				return -ERANGE;
			} else if (argument > EEPROM_SIZE_LIMIT) {
				fprintf(stderr, "Command line argument for -e is invalid.\n\n");
				return -EINVAL;
			}
			hargs->eesize = argument;
			break;

		case 'f':
			hargs->filename = optarg;
			break;

		default:
			return -EINVAL;
		}
	}

	return optind;
}
