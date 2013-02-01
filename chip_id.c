/*
 * AT91SAM7 Programmer, http://www.pjrc.com/arm/sam7_pgm
 * Copyright (c) 2005, PJRC.COM, LLC, <paul@pjrc.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
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
#include "chip_id.h"


static chipinfo_t known_chips[] = {
//                                       num                          set    clear
//                             num  page lock                         nvm    nvm
//  name           id_code     page size bits flasharea   ramarea     bits   bits
  {"AT91SAM7S256", 0x270B0940, 1024, 256, 16, 0x00100000, 0x00202000, 0x000, 0x000},
  {"AT91SAM7S256", 0x270D0940, 1024, 256, 16, 0x00100000, 0x00202000, 0x000, 0x000},
  {"AT91SAM7S128", 0x270A0740,  512, 256,  8, 0x00100000, 0x00202000, 0x000, 0x000},
  {"AT91SAM7S64",  0x27090540,  512, 128, 16, 0x00100000, 0x00202000, 0x000, 0x000},
  {"AT91SAM7S64",  0x27090544,  512, 128, 16, 0x00100000, 0x00202000, 0x000, 0x000},
  {"AT91SAM7S321", 0x27080342,  256, 128,  8, 0x00100000, 0x00202000, 0x000, 0x000},
  {"AT91SAM7S32",  0x27080340,  256, 128,  8, 0x00100000, 0x00202000, 0x000, 0x000},
  {"AT91SAM7X256", 0x275B0940, 1024, 256, 16, 0x00100000, 0x00202000, 0x004, 0x000},
  {"AT91SAM7X128", 0x275A0740,  512, 256,  8, 0x00100000, 0x00202000, 0x004, 0x000}
};

#define NUM_CHIPS (sizeof(known_chips) / sizeof(struct chip_info_struct))

chipinfo_t * get_chip_info(unsigned int id)
{
	int i;
	chipinfo_t *p = known_chips;

	for (i=0; i<NUM_CHIPS; i++) {
		if (p->id_code == id) return p;
		p++;
	}
	return NULL;
}

unsigned int chip_lock_mask(chipinfo_t *chip)
{
	if (chip == NULL) return 0;
	if (chip->num_lock_bits == 8) return 0x00FF;
	return 0xFFFF;
}



