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
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include "serial.h"
#include "cmd.h"
#include "settings.h"
#include "ihex.h"
#include "chip_id.h"

#define USAGE "sam7_pgm <uartdev> <firmware.hex>"

void die(const char *str)
{
	printf("%s\n", str);
	exit(1);
}

#define DBGU_BRGR	0xFFFFF220	/* Baud rate setting for debug uart */
#define DBGU_ID		0xFFFFF240	/* Chip ID code, see */
#define MC_FMR		0xFFFFFF60	/* Flash Mode Register */
#define MC_FCR		0xFFFFFF64	/* Flash Command Register */
#define MC_FSR		0xFFFFFF68	/* Flash Status Register */


int wait_flash_ready(void)
{
	int retry=0, r;
	unsigned int status;

	do {
		r = read_word(MC_FSR, &status);
		if (r) die("error while readying flash status");
		usleep(10000);
		if ((status & 1) == 1) return 0;
	} while (++retry < 100);
	return -1;
}

int set_flash_mode(double microseconds, double mclk)
{
	int r, clks;
	unsigned int mode;

	clks = (int)ceil(mclk * microseconds);
	printf("%d clocks per %.1f us\n", clks, microseconds * 1e6);
	mode = ((clks & 255) << 16) | 0x00000100;
	r = write_word(MC_FMR, mode);
	return r;
}


int unlock_flash_regions(chipinfo_t *chip, int mask, double mclk)
{
	int region, r;

	r = set_flash_mode(1.5e-6, mclk);
	if (r) die("Unable to set Flash Mode Register to 1.5us timing");

	for (region=0; region < chip->num_lock_bits; region++) {
		if (((mask >> region) & 1) == 0) continue;
		printf("Unlocking region #%d\n", region);
		r = wait_flash_ready();
		if (r) die("Timeout waiting for flash ready");
		r = write_word(MC_FCR, (region << 8) | 0x5A000004);
		if (r) die("error writing flash command");
	}
	r = wait_flash_ready();
	if (r) die("Timeout waiting for flash ready");
	return 0;
}


unsigned int word_from_firmware(unsigned int addr)
{
	return (((firmware_image[addr+3] & 255) << 24) |
		((firmware_image[addr+2] & 255) << 16) |
		((firmware_image[addr+1] & 255) << 8) |
		((firmware_image[addr+0] & 255)));
}


void wait(double t)
{
	while (t >= 0) {
		printf("  wait: %.1f  \r", t);
		fflush(stdout);
		t -= 0.1;
		usleep(100000);
	}
	printf("                  \r");
}


int main(int argc, char **argv)
{
	int r, i;
	char buf[256];
	unsigned int id, word, baud, lockmask;
	double mclk;
	int page, bit;
    char *fdev_name;
	chipinfo_t *chip;

	if (argc < 3)
        die(USAGE);

    fdev_name = argv[1];
	printf("opening serial port %s\n", fdev_name);
	open_serial_port(fdev_name);

	r = read_intel_hex(argv[2]);
	if (r < 0)
        die("Error reading intel hex file");
	printf("read %d bytes from %s\n", r, argv[2]);


	r = establish_comm_uart();
    if (r) {
			die("No response!  Unable to sync with boot agent.");
	}

	// read the bootloader ID and show it to the user
	r = bootloader_version(buf, sizeof(buf));
	if (r)
        die("Unable to read boot loader version");
	printf("Bootloader version: \"%s\"\n", buf);

	// read the chip ID
	r = read_word(DBGU_ID, &id);
	if (r) die("error while readying flash status");
	chip = get_chip_info(id);
	if (chip == NULL) {
		printf("Unknown chip ID is 0x%X\n", id);
		die("Contact paul@pjrc.com with info about this new chip!!");
	} else {
		printf("Chip is \"%s\"\n", chip->name);
	}

	// figure out what speed the CPU is running at
	r = read_word(DBGU_BRGR, &baud);
	if (r)
        die("Unable to read from chip");
	mclk = 115200.0 * 16.0 * (double)baud;
	printf("MCLK is approx %.3f MHz\n", mclk / 1e6);
	if (mclk < 19e6)
        printf("WARNING: below 19 MHz may be unreliable\n");
	if (mclk < 1e6)
        die("MCLK must be at least 1 MHz");
	if (mclk > 60e6)
        die("MCLK can not be over 60 MHz");

	// unlock flash regions, if necessary
	r = read_word(MC_FSR, &word);
	if (r)
        die("error while readying flash status");
	lockmask = (word >> 16) & chip_lock_mask(chip);
	if (lockmask) {
		printf("Unlocking flash regions\n");
		r = unlock_flash_regions(chip, lockmask, mclk);
		if (r)
            die("Unable to unlock flash regions");
		// todo: reread MC_FSR and verify it really did unlock all the regions
	}

	// change the flash config for writing to flash memory (1.0 us setting)
	// ...rather than lock bits which need a 1.5 us setting

	r = set_flash_mode(1.0e-6, mclk);
	if (r)
        die("Unable to set Flash Mode Register to 1us timing");

	// now program all the pages
	printf("Programming flash pages (%d bytes per page):\n", chip->pagesize);
	for (page=0; page < chip->numpages; page++) {
		// skip any pages not used by the code
		if (!bytes_within_range(page * chip->pagesize,
		   (page + 1) * chip->pagesize - 1)) {
			//printf("skipping page #%d, not used by code\n", page);
			continue;
		}

		printf(".");
		fflush(stdout);

		// make sure the flash is ready
		r = wait_flash_ready();
		if (r) die("Timeout waiting for flash ready");

		// write a page into the flash buffer
		for (i=0; i<chip->pagesize; i+=4) {
			r = write_word(chip->flasharea + page * chip->pagesize + i,
				word_from_firmware(page * chip->pagesize + i));
			if (r) die("error writing word\n");
		}

		// and tell the flash controller to burn it into the flash memory!
		r = write_word(MC_FCR, (page << 8) | 0x5A000001);
		if (r)
            die("error writing flash command\n");
	}
	r = wait_flash_ready();
	if (r) die("Timeout waiting for flash ready");
	printf("\n");

	if (chip->set_nvm_bits) {
		for (bit=0; bit<20; bit++) {
			if (chip->set_nvm_bits & (1 << bit)) {
				printf("Setting General Purpose NVM bit #%d\n", bit);
				// make sure the flash is ready
				r = wait_flash_ready();
				if (r) die("Timeout waiting for flash ready");
				// and tell the flash controller to burn it into the flash memory!
				r = write_word(MC_FCR, (bit << 8) | 0x5A00000B);
				if (r) die("error writing Set NVM bit command\n");
			}
		}
	}

	if (chip->clear_nvm_bits) {
		for (bit=0; bit<20; bit++) {
			if (chip->clear_nvm_bits & (1 << bit)) {
				printf("Clearing General Purpose NVM bit #%d\n", bit);
				// make sure the flash is ready
				r = wait_flash_ready();
				if (r) die("Timeout waiting for flash ready");
				// and tell the flash controller to burn it into the flash memory!
				r = write_word(MC_FCR, (bit << 8) | 0x5A00000D);
				if (r) die("error writing Clear NVM bit command\n");
			}
		}
	}

	r = run_code(0);
	if (r == 0) {
		printf("Running your new code...\n");
	} else {
		printf("Error jumping to your new code, sorry!\n");
	}

	return 0;
}

