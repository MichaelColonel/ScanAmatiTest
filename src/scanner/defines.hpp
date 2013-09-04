/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#pragma once

#define SCANNER_DATA_TIMEOUT                5 // in seconds
#define SCANNER_FT245_BUFFER_SIZE           256 // in bytes

#define SCANNER_MEMORY_IMAGE        256 * 16 * 16 * 96 // in bytes
#define SCANNER_MEMORY_BANK         (1 << 23) // in bytes
#define SCANNER_MEMORY              (1 << 25) // in bytes
#define SCANNER_MEMORY_CHUNK        (1 << 19) // in bytes
#define SCANNER_MEMORY_PART         (1 << 17) // in bytes
#define SCANNER_MEMORY_ALL          (SCANNER_MEMORY + SCANNER_MEMORY_PART)

#define SCANNER_ADC_RESOLUTION      14
#define SCANNER_ADC_COUNT_MIN       0
#define SCANNER_ADC_COUNT_MAX       ((1 << SCANNER_ADC_RESOLUTION) - 1)
#define SCANNER_ADC_COUNTS          (1 << SCANNER_ADC_RESOLUTION)

#define SCANNER_CHIPS                 16

#define SCANNER_CHIPS_DROP_HALF       4
#define SCANNER_CHIPS_DROP_QUARTER    6

#define SCANNER_LINING_CODE_MIN       0
#define SCANNER_LINING_CODE_MAX       255
#define SCANNER_LINING_CODES          256

#define SCANNER_LINING_COUNT          1000

#define SCANNER_STRIPS_PER_CHIP       132
#define SCANNER_STRIPS_PER_CHIP_REAL  128
#define SCANNER_STRIPS       (SCANNER_STRIPS_PER_CHIP * SCANNER_CHIPS)
#define SCANNER_STRIPS_REAL  (SCANNER_STRIPS_PER_CHIP_REAL * SCANNER_CHIPS)

#define SCANNER_DROP_STRIPS_PER_CHIP  4
#define SCANNER_DROP_STRIPS (SCANNER_DROP_STRIPS_PER_CHIP * SCANNER_CHIPS)

#define SCANNER_HEADER          8  // header size of scanner buffer
#define SCANNER_BUFFER         72  // buffer size
#define SCANNER_BUFFER_FULL   256
#define SCANNER_BUFFER_PART    16
#define SCANNER_BUFFER_WELCOME  7

#define IMAGE_STRIPS_PER_CHIP \
	(SCANNER_STRIPS_PER_CHIP - SCANNER_DROP_STRIPS_PER_CHIP)

#define IMAGE_STRIPS   (SCANNER_STRIPS - SCANNER_DROP_STRIPS)
#define IMAGE_ROWS     (((SCANNER_MEMORY >> 1) / SCANNER_STRIPS) - 2)

#define IMAGE_WIDTH    IMAGE_STRIPS
//#define IMAGE_HEIGHT   1578
#define IMAGE_HEIGHT   1520
//#define IMAGE_HEIGHT   (((SCANNER_MEMORY_IMAGE >> 1) / SCANNER_STRIPS) - 2)
