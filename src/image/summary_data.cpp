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

#include "summary_data.hpp"

// files from src directory begin
#include "scanner/defines.hpp"
// files from src directory end

namespace ScanAmati {

namespace Image {

bool
SummaryData::check_available_data(DataType type) const
{
	DataSharedPtr data;
	switch (type) {
	case DATA_RAW:
		data = raw_data();
		break;
	case DATA_FOR_PROCESSING:
		data = processing_data();
		break;
	case DATA_FOR_PRESENTATION:
		data = presentation_data();
		break;
	default:
		break;
	}

	return (data) ? true : false;
}

DataSharedPtr
SummaryData::get_image(DataType type) const
{
	DataSharedPtr data;
	switch (type) {
	case DATA_RAW:
		data = raw_data();
		break;
	case DATA_FOR_PROCESSING:
		data = processing_data();
		break;
	case DATA_FOR_PRESENTATION:
		data = presentation_data();
		break;
	default:
		break;
	}

	return (data);
}

bool
SummaryData::fill_image_buffer(DataType type)
{
	bool res = false;
	Image::DataSharedPtr image;

	switch (type) {
	case DATA_RAW:
		image = raw_data();
		break;
	case DATA_FOR_PROCESSING:
		image = processing_data();
		break;
	case DATA_FOR_PRESENTATION:
		image = presentation_data();
		break;
	default:
		break;
	}

	if (image) {
		std::vector<guint8>& buf = image_buffer();
		buf.resize(image->width() * image->height());
		for ( const gint16* pos = image->begin(); pos != image->end(); ++pos)
		{
			ptrdiff_t i = pos - image->begin();
			double value = double(UCHAR_MAX * *pos) / SCANNER_ADC_COUNT_MAX;
			buf[i] = static_cast<gint8>(value);
		}
		res = true;
	}
	return res;
}

} // namespace Image

} // namespace ScanAmati
