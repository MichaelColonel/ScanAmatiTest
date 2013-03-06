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

#include <map>
#include "image_data.hpp"

namespace ScanAmati {

namespace Image {

typedef std::map< double, DataVector> CalibrationMap;
typedef CalibrationMap::value_type CalibrationPair;

class Calibration {

public:
	Calibration( CalibrationMap& map, const std::vector<guint>& skip,
		guint range, double tension = 200.)
		: map_(map), skip_(skip), range_(range), tension_(tension) {}
	double add_row( const DataVector& row, bool skip_bad_strips = true);
	void add_row( double value, const DataVector& row);
	void add_row( double value, const DataSharedPtr& data,
		unsigned int lower, unsigned int upper);
	void expand( double min, double max);
	DataSharedPtr calibrate(const DataSharedPtr&);
private:
	double mean_skip(const DataVector& mean_vector);

	CalibrationMap& map_;
	const std::vector<guint>& skip_;
	guint range_;
	double tension_;
};

} // namespace Image

} // namespace ScanAmati
