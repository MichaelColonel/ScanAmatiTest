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

#include "file.hpp"

namespace ScanAmati {

bool
File::write_image_data( DcmDataset* dataset,
	const Image::DataSharedPtr& image)
{
	OFCondition status;
	Uint16* data = reinterpret_cast<Uint16*>(image->data());

	status = dataset->putAndInsertUint16( DCM_Rows, image->height());
	if (!status.good())
		return false;

	status = dataset->putAndInsertUint16( DCM_Columns, image->width());
	if (!status.good())
		return false;
	
	if (data) {
		status = dataset->putAndInsertUint16Array( DCM_PixelData, data,
			image->width() * image->height());
		if (!status.good())
			return false;
	}
	return true;
}

} // namespace ScanAmati
