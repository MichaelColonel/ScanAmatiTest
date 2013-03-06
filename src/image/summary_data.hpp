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

#include <tr1/tuple>

#include "data.hpp"
#include "calibration.hpp"

namespace ScanAmati {
	
namespace Image {

typedef std::tr1::tuple<
	std::vector<guint8>, // memory buffer
	DataSharedPtr, // raw data
	DataSharedPtr, // data for processing
	DataSharedPtr, // data for presentation
	CalibrationMap, // calibration data without expansion
	std::vector<guint8> // image buffer
> SummaryTuple;

enum DataType {
	DATA_RAW,
	DATA_FOR_PROCESSING,
	DATA_FOR_PRESENTATION
};

class SummaryData {
public:
	SummaryData() {}
	SummaryData(const SummaryTuple& tuple) : tuple_(tuple) {}
	std::vector<guint8>& memory() { return std::tr1::get<0>(tuple_); }
	DataSharedPtr& raw_data() { return std::tr1::get<1>(tuple_); }
	const DataSharedPtr& raw_data() const { return std::tr1::get<1>(tuple_); }
	DataSharedPtr& processing_data() { return std::tr1::get<2>(tuple_); }
	const DataSharedPtr& processing_data() const { return std::tr1::get<2>(tuple_); }
	DataSharedPtr& presentation_data() { return std::tr1::get<3>(tuple_); }
	const DataSharedPtr& presentation_data() const { return std::tr1::get<3>(tuple_); }
	CalibrationMap& calibration_data() { return std::tr1::get<4>(tuple_); }
	std::vector<guint8>& image_buffer() { return std::tr1::get<5>(tuple_); }
	std::vector<guint8> image_buffer() const { return std::tr1::get<5>(tuple_); }
	DataSharedPtr get_image(DataType type = DATA_FOR_PRESENTATION) const;
	bool fill_image_buffer(DataType);
	bool check_available_data(DataType) const;

protected:
	SummaryTuple tuple_;
};

} // namespace Image

} // namespace ScanAmati
