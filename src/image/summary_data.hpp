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
	DataSharedPtr, // raw data
	std::vector<guint8> // image buffer
> SummaryTuple;

class SummaryData {
public:
	SummaryData() {}
	SummaryData(const SummaryTuple& tuple) : tuple_(tuple) {}
	DataSharedPtr& raw_data() { return std::tr1::get<0>(tuple_); }
	const DataSharedPtr& raw_data() const { return std::tr1::get<0>(tuple_); }
	std::vector<guint8>& image_buffer() { return std::tr1::get<1>(tuple_); }
	std::vector<guint8> image_buffer() const { return std::tr1::get<1>(tuple_); }
	bool fill_image_buffer();

protected:
	SummaryTuple tuple_;
};

} // namespace Image

} // namespace ScanAmati
