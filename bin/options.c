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
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "options.h"

#define EEPROM_SIZE_LIMIT	4096

static int conv_ul_arg(size_t *dest, const char *arg, size_t min, size_t max,
	char name)
{
	unsigned long num;

	if (!dest)
		return -EINVAL;

	errno = 0;
	num = strtoul(arg, NULL, 0);
	if (errno) {
		if (name)
			fprintf(stderr, "Invalid parameter for -%c: '%s'\n",
				name, arg);
		return -errno;
	}

	if (num < min || num > max) {
		if (name)
			fprintf(stderr, "Parameter for -%c out of range "
				"[%lu, %lu]\n", name, min, max);
		return -ERANGE;
	}

	*dest = num;

	return 0;
}

void options_help(const char *progname)
{
	fprintf(stderr,
		"Usage: %s [{-b BUSNUM -d DEVNUM}] [-v] [-l]\n"
		"          [-P PORT] [{-p [VALUE]|-i [VALUE]}]\n\n"
		"or:    %s [{-b BUSNUM -d DEVNUM}] [-v]\n"
		"          [{-w BYTES -f filename} | {-r BYTES -f filename} | -e BYTES] [-x]\n\n"
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
		"-V                     show program version and quit\n"
		"-w     <N>             Write N bytes to EEPROM\n"
		"-x                     Overwrite non-blank EEPROM devices\n",
		progname, progname);
}

int options_scan(struct hub_options *hargs, int argc, char **argv)
{
	const char short_options[] = "b:d:e:f:hi:lP:p:qr:Vvw:x";
	int option;
	int ret;

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
			ret = conv_ul_arg(&hargs->busnum, optarg, 1, USHRT_MAX,
				option);
			if (ret)
				return ret;
			break;

		case 'd':
			ret = conv_ul_arg(&hargs->devnum, optarg, 1, USHRT_MAX,
				option);
			if (ret)
				return ret;
			break;

		case 'P':
			if (hargs->cmd != COMMAND_SET_NONE &&
					hargs->cmd != COMMAND_SET_POWER)
				return -EINVAL;

			ret = conv_ul_arg(&hargs->port, optarg, 1, USHRT_MAX,
				option);
			if (ret)
				return ret;

			hargs->cmd = COMMAND_SET_POWER;
			break;

		case 'i':
			if (hargs->cmd != COMMAND_SET_NONE)
				return -EINVAL;

			ret = conv_ul_arg(&hargs->power, optarg, 0, 3, option);
			if (ret)
				return ret;

			hargs->cmd = COMMAND_SET_LED;
			break;

		case 'p':
			if (hargs->cmd != COMMAND_SET_NONE &&
					hargs->cmd != COMMAND_SET_POWER)
				return -EINVAL;

			ret = conv_ul_arg(&hargs->power, optarg, 0, 1, option);
			if (ret)
				return ret;

			hargs->cmd = COMMAND_SET_POWER;
			break;

		case 'v':
			hargs->verbose = 1;
			break;

		case 'q':
			hargs->quiet = 1;
			break;

		case 'r':
		case 'w':
		case 'e':
			if (hargs->cmd != COMMAND_SET_NONE)
				return -EINVAL;

			ret = conv_ul_arg(&hargs->eesize, optarg, 1,
				EEPROM_SIZE_LIMIT, option);
			if (ret)
				return ret;

			hargs->cmd =
				option == 'r' ? COMMAND_GET_EEPROM :
				option == 'w' ? COMMAND_SET_EEPROM :
				COMMAND_CLR_EEPROM;
			break;

		case 'x':
			hargs->overwrite = 1;
			break;

		case 'f':
			hargs->filename = optarg;
			break;

		case 'V':
			hargs->version = 1;
			return 0;

		default:
			return -EINVAL;
		}
	}

	return optind;
}
