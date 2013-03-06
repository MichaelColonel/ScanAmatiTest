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
#include <cstddef> // size_t
#include <vector>
#include <boost/any.hpp>

#include "movement.hpp"
#include "data.hpp"
#include "commands.hpp"

namespace ScanAmati {

namespace Scanner {

struct Acquisition {
	Acquisition();
	Acquisition( unsigned int height, size_t size,
		const Movement& forward, const Movement& reverse);

	unsigned int image_height;
	size_t memory_size;
	Movement movement_forward;
	Movement movement_reverse;
};

struct AcquisitionParameters {
	AcquisitionParameters();
	AcquisitionParameters(Command* com);
	AcquisitionParameters(const std::vector<Command*>& coms);
	~AcquisitionParameters();
	bool with_acquisition;
	bool with_exposure;
	MovementType movement_type;
	Magick::FilterTypes filter_type;
	WidthType width_type;
	CalibrationType calibration_type;
	PixelIntensityType intensity_type;
	LiningAccuracyType lining_accuracy_type;
	gint16 lining_count;
	Acquisition acquisition;
	std::vector<CommandSharedPtr> commands;
	boost::any value; // chip capasity, chip number
};

} // namespace Scanner

} // namespace ScanAmati
