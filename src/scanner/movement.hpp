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

namespace Scanner {

enum DirectionType {
	DIRECTION_FORWARD,
	DIRECTION_REVERSE
};

enum MovementType {
	MOVEMENT_NONE,
	MOVEMENT_ARRAY,
	MOVEMENT_XRAY,
	MOVEMENT_BOTH
};

const struct Movement {
	double time;
	int steps;
	struct Speed {
		unsigned short freq; // frequency
		bool mode; // 0 - Full Step, 1 - Half Step
	};
	Speed array_speed;
	Speed xray_speed;
	unsigned char xray_delay;
	unsigned char xray_array_delay;
} system_movements[] = {
	{ // 27 cm
		4.3, // time
		3250, // steps
		{ (120 << 8) | 20, 0 }, // array frequency and mode
		{ 216, 0 }, // xray frequency and mode
		237, // xray delay
		230 // xray-array delay
	},
	{ // 37 cm
		4.3, // time
		3250, // steps
		{ (107 << 8) | 220, 0 }, // scanner frequency and mode
		{ 221, 0 }, // xray frequency and mode
		237, // xray delay
		230 // xray-scanner delay
	},
	{ } // Terminating entry
};

} // namespace Scanner

} // namespace ScanAmati
