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

#include "xmodem.h"
#include "serial.h"

#define SOH 0x01
#define EOT 0x04
#define ACK 0x06
#define NAK 0x15
#define CAN 0x18

static int xmodem_crc(unsigned char *p, int len);

void print_packet(const unsigned char *p, int len)
{
	int i;

	printf("packet data: \n");
	for (i=0; i<133; i++) {
		printf("%02X ", p[i]);
		if ((i + 1) % 16 == 0) printf("\n");
	}
	printf("\n");
}

int xmodem_transmit(const unsigned char *data, int len)
{
	char c;
	int r, retry=0, crc;
	int packet_num, total_packets;
	unsigned char packet[136], buf[16];

	//r = read_serial_port_nb(buf, sizeof(buf));
	//printf("r = %d\n", r);

	do {
		r = read_serial_port_timeout(buf, sizeof(buf), NULL, 0.1);
		printf("r=%d, c=%d\n", r, c);
		if (r > 0 && strchr((char *)buf, 'C')) break;
	} while (++retry < 10);
	if (retry >= 10) {
		return -1;
	}
	printf("Got 'C', starting Xmodem transfer\n");

	total_packets = (len + 127) / 128;

	for (packet_num=0; packet_num<total_packets; packet_num++) {
		retry = 0;
		packet[0] = 1;
		packet[1] = (packet_num + 1) & 255;
		packet[2] = 255 - packet[1];
		if ((packet_num + 1) * 128 <= len) {
			//printf("full 128 byte packet\n");
			memcpy(packet + 3, data + packet_num * 128, 128);
		} else {
			//printf("short packet, %d bytes\n", len - packet_num * 128);
			memset(packet + 3, 0, 128);
			memcpy(packet + 3, data + packet_num * 128,
				len - packet_num * 128);
		}
		crc = xmodem_crc(packet + 3, 128);
		packet[131] = (crc >> 8) & 255;
		packet[132] = crc & 255;
		//print_packet(packet, 133);

		do {
			printf("sending packet #%d\n", packet_num + 1);
			write_serial_port(packet, 133);
			r = read_serial_port_timeout((unsigned char *)&c, 1, NULL, 0.2);
			printf("r=%d, c=%d\n", r, c);
			if (r != 1) return -3;		// no response
			if (c == ACK) break;
			if (c != NAK) return -4;	// only ACK and NAK allowed
		} while (++retry < 10);
		if (retry >= 10) {
			return -2;
		}
	}
	c = EOT;
	write_serial_port((unsigned char *)&c, 1);
	r = read_serial_port_timeout((unsigned char *)&c, 1, NULL, 0.2); // final ack
	//printf("final ack: r=%d, c=%d\n", r, c);


	return 0;
}



static int check_packet(unsigned char *pkt, int pkt_num)
{
	int crc;

	if (pkt[0] != 0x01) return -1;
	if (pkt[1] != pkt_num) return -2;
	if (pkt[2] != 255 - pkt_num) return -3;
	crc = xmodem_crc(pkt + 3, 128);
	if (crc != ((pkt[131] << 8) | pkt[132])) return -4;
	return 0;
}


int xmodem_receive(unsigned char *buf, int bufsize)
{
	int r, num=0, i;
	unsigned char c, pkt_num=1;
	unsigned char packet[256];

	write_serial_port("C", 1);

	while (1) {
		r = read_serial_port_timeout(packet, 1, NULL, 0.25);
		if (r != 1) {
			printf("error reading from serial port\n");
			return -1;
		}
		if (r == 1 && packet[0] == EOT) {
			printf("xmodem: received EOT\n");
			c = ACK;
			write_serial_port(&c, 1);
			return num;
		}
		r = read_serial_port_timeout(packet + 1, 132, NULL, 0.1);
		if (r < 0) {
			printf("error reading from serial port\n");
			return -1;
		}

		printf("received %d chars:\n", r+1);
		for (i=0; i<r+1; i++) {
			printf("%02X ", packet[i]);
			if ((i + 1) % 16 == 0) printf("\n");
		}
		printf("\n");

		if (r < 132 || check_packet(packet, pkt_num) != 0) {
			// incomplete data or bad packet
			printf("xmodem: error in pkt %d, r=%d\n", pkt_num, check_packet(packet, pkt_num));
			c = NAK;
			write_serial_port(&c, 1);
		} else {
			// good packet received
			printf("xmodem: pkt %d received ok\n", pkt_num);
			num += 128;
			if (num <= bufsize) {
				memcpy(buf, packet+3, 128);
			}
			c = ACK;
			write_serial_port(&c, 1);
			pkt_num++;
		}
	}
}




static int xmodem_crc(unsigned char *p, int len)
{
	int i;
	unsigned short crc=0;

	while (len--) {
		crc = crc ^ (unsigned short)*p++ << 8;
		for (i=0; i<8; i++) {
			if (crc & 0x8000)
				crc = crc << 1 ^ 0x1021;
			else
				crc = crc << 1;
		}
	}
	return crc;
}




#if 0
static unsigned short crc16_table[256];

//#define POLY 0x8408	// ppp
//#define POLY 0xA001	// modbus

void crc16_table_init(void)
{
	int i, j, val;

	for (i=0; i<256; i++) {
		val = i;
		for (j=0; j<8; j++) {
			val = val & 1 ? (val >> 1) ^ POLY : val >> 1;
		}
		crc16_table[i] = val;
	}
}

int crc16(unsigned char *p, int len, int seed)
{
	unsigned short crc=(unsigned short)seed;
	while (len--) {
		crc = (crc >> 8) ^ crc16_table[(crc ^ *p++) & 255];
	}
	return crc;
}
#endif


