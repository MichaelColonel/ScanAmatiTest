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

#include <medcon.h>

#include "palette.hpp"

namespace ScanAmati {

namespace Image {

Palette*
Palette::create(PaletteType type)
{
	Palette* palette = new Palette;
	palette->fill(type);
	return palette;
}

void
Palette::fill(PaletteType type)
{
	switch (type) {
	case PALETTE_INVERTED:
		MdcInvertedScale(rgb_);
		break;
	case PALETTE_RAINBOW:
		MdcRainbowScale(rgb_);
		break;
	case PALETTE_COMBINED:
		MdcCombinedScale(rgb_);
		break;
	case PALETTE_HOTMETAL:
		MdcHotmetalScale(rgb_);
		break;
	case PALETTE_GRAYSCALE:
	default:
		MdcGrayScale(rgb_);
		break;
	}
}

} // namespace Image

} // namespace ScanAmati
