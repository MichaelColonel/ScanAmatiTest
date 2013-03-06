/*
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *      
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *      
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */

#pragma once

#include <config.h>

#ifdef HAVE_GLIB_2_0
#include <glib.h>
#endif

#include "defines.hpp"

namespace ScanAmati {
	
namespace Scanner {

union AdcCount {
	AdcCount(guint16 v = 0) : value(v) {}
	AdcCount( guint8 l = 0, guint8 h = 0) { byte.low = l; byte.high = h; }
	guint16 temperature_code() const { return (byte.low << 8) | byte.high; }
	gint16 pixel() const;
#if SCANNER_ADC_RESOLUTION == 14
	bool data_bit() const { return byte.low & 0x80; }
	bool chip_bit() const { return byte.low & 0x40; }
#elif SCANNER_ADC_RESOLUTION == 12
	bool data_bit() const { return byte.low & 0x40; }
	bool chip_bit() const { return byte.low & 0x20; }
	bool test_bit() const { return byte.low & 0x10; }
#endif
	guint16 value;
	struct {
		guint8 low;
		guint8 high;
	} byte;
};

} // namespace Scanner

} // namespace ScanAmati
