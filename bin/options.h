/**
 * @file
 * @author Bert van Hall <bert.vanhall\@avionic-design.de>
 * @date 2016
 *
 * @brief Options parsing for hub-ctrl
 *
 * @copyright GPLv3
 */

#ifndef OPTIONS_H
#define OPTIONS_H

#include <stdint.h>

#define COMMAND_SET_NONE		0
#define COMMAND_SET_LED			(1 << 0)
#define COMMAND_SET_POWER		(1 << 1)
#define COMMAND_GET_EEPROM		(1 << 2)
#define COMMAND_SET_EEPROM		(1 << 3)
#define COMMAND_CLR_EEPROM		(1 << 4)

struct hub_options {
	int cmd;
	char *filename;
	uint16_t erase_size;
	uint16_t write_size;
	uint16_t read_size;
	int busnum;
	int devnum;
	int index;
	int power;
	int port;
	int verbose;
	int listing;
	int quiet;
};

void options_help(const char *progname);

int options_scan(struct hub_options *hargs, int argc, char **argv);;

#endif /* OPTIONS_H */
