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

#include <stddef.h>

#define COMMAND_SET_NONE		0
#define COMMAND_SET_LED			(1 << 0)
#define COMMAND_SET_POWER		(1 << 1)
#define COMMAND_GET_EEPROM		(1 << 2)
#define COMMAND_SET_EEPROM		(1 << 3)
#define COMMAND_CLR_EEPROM		(1 << 4)
#define COMMAND_TYPE_EEPROM		\
		( COMMAND_GET_EEPROM | COMMAND_SET_EEPROM | COMMAND_CLR_EEPROM )

struct hub_options {
	int cmd;
	char *filename;
	size_t eesize;
	size_t busnum;
	size_t devnum;
	size_t power;
	size_t port;
	int overwrite;
	int verbose;
	int listing;
	int quiet;
	char version;
};

void options_help(const char *progname);

int options_scan(struct hub_options *hargs, int argc, char **argv);;

#endif /* OPTIONS_H */
