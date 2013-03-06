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

namespace ScanAmati {

namespace Image {

enum PaletteType {
	PALETTE_GRAYSCALE,
	PALETTE_INVERTED,
	PALETTE_RAINBOW,
	PALETTE_COMBINED,
	PALETTE_HOTMETAL
};

class Palette {
public:
	static Palette* create(PaletteType type = PALETTE_GRAYSCALE);
	void fill(PaletteType);
	unsigned char rgb(unsigned int i) const { return rgb_[i]; }
private:
	enum SizeType { IMAGE_PALETTE_SIZE = 768 }; // 3 * 256
	unsigned char rgb_[IMAGE_PALETTE_SIZE];
};

} // namespace Image

} // namespace ScanAmati
