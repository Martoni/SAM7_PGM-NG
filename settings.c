/*
 * AT91SAM7 Programmer, http://www.pjrc.com/arm/sam7_pgm
 * Copyright (c) 2005, PJRC.COM, LLC, <paul@pjrc.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */


// Edit these!!!
// (todo: someday make these configurable)
#define DEFAULT_PORT "/dev/ttyUSB0"
#define DEFAULT_BAUD "115200"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "settings.h"


char *baud_list[]={"115200", "57600", "38400",
       "19200", "9600", "4800", "2400", "1200", "300", NULL};

static char port[64]={DEFAULT_PORT};
static char baud[64]={DEFAULT_BAUD};

static char settings_file[256]={'\0'};


void init_settings(void)
{
	const char *home_dir;
	FILE *fp;
    char *cret;
	char buf[1024], *p, *q;

	home_dir = getenv("HOME");
	if (home_dir && *home_dir) {
		snprintf(settings_file, sizeof(settings_file),
			"%s/.sam7_pgm", home_dir);
		fp = fopen(settings_file, "r");
		if (fp == NULL) return;
		while (!feof(fp)) {
			buf[0] = '\0';
			cret = fgets(buf, sizeof(buf), fp);
            if( cret == NULL) {
                printf("%s: error in fgets\n", __FUNCTION__);
                return;
            }
			if (strncmp(buf, "port:", 5) == 0) {
				for (p=buf+5; isspace(*p); p++) ;
				q = rindex(p, '\n'); if (q) *q = '\0';
				q = rindex(p, '\r'); if (q) *q = '\0';
				snprintf(port, sizeof(port), "%s", p);
			}
			if (strncmp(buf, "baud:", 5) == 0) {
				for (p=buf+5; isspace(*p); p++) ;
				q = rindex(p, '\n'); if (q) *q = '\0';
				q = rindex(p, '\r'); if (q) *q = '\0';
				snprintf(baud, sizeof(baud), "%s", p);
			}
		}
		fclose(fp);
	}
}

void write_settings_file(void)
{
	FILE *fp;

	if (settings_file[0] == '\0') return;
	fp = fopen(settings_file, "w");
	if (fp == NULL) return;
	fprintf(fp, "port: %s\n", port);
	fprintf(fp, "baud: %s\n", baud);
	fflush(fp);
	fclose(fp);
}

const char * baud_setting(void)
{
	return baud;
}

void new_port_setting(const char *new_port)
{
	if (strcmp(port, new_port)) {
		snprintf(port, sizeof(port), "%s", new_port);
		write_settings_file();
	}
}

void new_baud_setting(const char *new_baud)
{
	if (strcmp(baud, new_baud)) {
		snprintf(baud, sizeof(baud), "%s", new_baud);
		write_settings_file();
	}
}


