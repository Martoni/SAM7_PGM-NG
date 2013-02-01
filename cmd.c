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


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "settings.h"
#include "serial.h"
#include "xmodem.h"


int establish_comm_uart(void)
{
	char buf[256];
	unsigned char c80=0x80;
	int n;

	input_flush_serial_port();
	write_serial_port(&c80, 1);
	usleep(5000);
	write_serial_port(&c80, 1);
	usleep(5000);
	write_serial_port("#", 1);
	n = read_serial_port_timeout((unsigned char *)buf, sizeof(buf)-1, ">", 0.2);
	if (n < 0) {
		printf("error reading, n=%d\n", n);
		return -1;
	} else {
		buf[n] = '\0';
		//  uncomment this to "see" what is coming back from
		//  the boot agent sync, if anything at all
		//printf("received %d chars: %s\n", n, buf);
		if (n > 0 && buf[n-1] == '>') return 0;
	}
	return -2;
}


int bootloader_version(char *str, int str_size)
{
	char buf[256];
	int n, i, j;

	*str = '\0';
	input_flush_serial_port();
	write_serial_port("V#", 2);
	n = read_serial_port_timeout((unsigned char *)buf, sizeof(buf)-1, ">", 0.2);
	if (n < 0) {
		printf("error reading, n=%d\n", n);
		return -1;
	} else {
		if (n > 0 && buf[n-1] == '>') {
			// copy bootloader string, but not CR or NL, or final '>'
			n--;
			if (n > str_size - 1) n = str_size - 1;
			for (i=j=0; i<n; i++) {
				if (buf[i] == '\r' || buf[i] == '\n') continue;
				str[j++] = buf[i];
			}
			str[j] = '\0';
			return 0;
		}
	}
	return -2;
}


int run_code(unsigned int addr)
{
	char buf[256];
	int n;

	input_flush_serial_port();
	snprintf(buf, sizeof(buf), "G%X#", addr);
	printf("sending command \"%s\"\n", buf);
	write_serial_port(buf, strlen(buf));

	n = read_serial_port_timeout((unsigned char *)buf, sizeof(buf)-1, ">", 1.5);
	if (n < 0) {
		printf("error reading, n=%d\n", n);
		return -1;
	} else {
		buf[n] = '\0';
		//printf("received %d chars: %s\n", n, buf);
		if (n > 0 && buf[n-1] == '>') return 0;
		if (addr == 0 && (buf[0] == 13 || buf[0] == 10)) return 0;
	}
	return -2;
}



int write_word(unsigned int addr, unsigned int word)
{
	char buf[256];
	int n;

	input_flush_serial_port();
	snprintf(buf, sizeof(buf), "W%X,%X#", addr, word);
	write_serial_port(buf, strlen(buf));
	n = read_serial_port_timeout((unsigned char *)buf, sizeof(buf)-1, ">", 0.2);
	if (n < 0) {
		printf("error reading, n=%d\n", n);
		return -1;
	} else {
		buf[n] = '\0';
		//printf("received %d chars: %s\n", n, buf);
		if (n > 0 && buf[n-1] == '>') {
			//printf("write %08X to location %08X\n", word, addr);
			return 0;
		}
	}
	return -2;
}


int read_word(unsigned int addr, unsigned int * word)
{
	char buf[256], *p;
	int n, r;

	input_flush_serial_port();
	snprintf(buf, sizeof(buf), "w%X,#", addr);
	write_serial_port(buf, strlen(buf));
	n = read_serial_port_timeout((unsigned char *)buf, sizeof(buf)-1, ">", 0.2);
	if (n < 0) {
		printf("error reading, n=%d\n", n);
		return -1;
	} else {
		buf[n] = '\0';
		//printf("received %d chars: %s\n", n, buf);
		if (n > 0 && buf[n-1] == '>') {
			p = buf;
			while (isspace(*p)) p++;
			r = sscanf(p, "0x%x", word);
			if (r == 1) {
				//printf("read %08X at location %08X\n", *word, addr);
				return 0;
			} else {
				return -3;
			}
		}

	}
	return -2;
}

int read_memory(int addr, int len)
{
	int r;
	unsigned char memory[0x200000];

	write_serial_port("R000000,180#", 12);
	usleep(20000);
	input_flush_serial_port();
	r = xmodem_receive(memory, sizeof(memory));
	return r;
}


int write_memory(int addr, const unsigned char *data, int len)
{
	int r;
	char buf[256];

	input_flush_serial_port();
	snprintf(buf, sizeof(buf), "S%X,%X#", addr, len);
	printf("sending command \"%s\"\n", buf);
	write_serial_port(buf, strlen(buf));
	r = xmodem_transmit(data, len);
	return r;
}



