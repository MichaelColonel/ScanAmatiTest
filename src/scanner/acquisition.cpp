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

/* files from src directory begin */
#include "global_strings.hpp"
/* files from src directory end */

#include "defines.hpp"
#include "acquisition.hpp"

namespace ScanAmati {

namespace Scanner {

Acquisition::Acquisition()
	:
	image_height(IMAGE_HEIGHT),
	memory_size(SCANNER_MEMORY),
	movement_forward(system_movements[1]),
	movement_reverse(system_movements[1])
{
}

Acquisition::Acquisition( unsigned int height, size_t size,
	const Movement& forward, const Movement& reverse)
	:
	image_height(height),
	memory_size(size),
	movement_forward(forward),
	movement_reverse(reverse)
{
}

AcquisitionParameters::AcquisitionParameters()
	:
	with_acquisition(true),
	with_exposure(true),
	movement_type(MOVEMENT_BOTH),
	filter_type(Magick::LanczosFilter),
	width_type(Scanner::WIDTH_FULL),
	calibration_type(Scanner::CALIBRATION_ROUGH),
	intensity_type(Scanner::INTENSITY_ORIGINAL),
	lining_accuracy_type(Scanner::LINING_ACCURACY_OPTIMAL),
	lining_count(SCANNER_LINING_COUNT),
	acquisition()
{
}

AcquisitionParameters::AcquisitionParameters(Command* com)
	:
	with_acquisition(true),
	with_exposure(true),
	movement_type(MOVEMENT_BOTH),
	filter_type(Magick::LanczosFilter),
	width_type(Scanner::WIDTH_FULL),
	calibration_type(Scanner::CALIBRATION_ROUGH),
	intensity_type(Scanner::INTENSITY_ORIGINAL),
	lining_accuracy_type(Scanner::LINING_ACCURACY_OPTIMAL),
	lining_count(SCANNER_LINING_COUNT),
	acquisition()
{
	commands.push_back(CommandSharedPtr(com));
}

AcquisitionParameters::AcquisitionParameters(const std::vector<Command*>& coms)
	:
	with_acquisition(true),
	with_exposure(true),
	movement_type(MOVEMENT_BOTH),
	filter_type(Magick::LanczosFilter),
	width_type(Scanner::WIDTH_FULL),
	calibration_type(Scanner::CALIBRATION_ROUGH),
	intensity_type(Scanner::INTENSITY_ORIGINAL),
	lining_accuracy_type(Scanner::LINING_ACCURACY_OPTIMAL),
	lining_count(SCANNER_LINING_COUNT),
	acquisition()
{
	for ( std::vector<Command*>::const_iterator it = coms.begin();
		it != coms.end(); ++it) {
		commands.push_back(CommandSharedPtr(*it));
	}
}

AcquisitionParameters::~AcquisitionParameters()
{
}

} // namespace Scanner

} // namespace ScanAmati
