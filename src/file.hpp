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

#include "image/summary_data.hpp"
#include "dicom/summary_information.hpp"

namespace ScanAmati {

class File {

public:
	File() {}
	File(const DICOM::SummaryInfo& dicom) : dicom_info_(dicom) {}
	File(const Image::SummaryData& image) : image_data_(image) {}
	File( const Image::SummaryData&, const DICOM::SummaryInfo&);
	~File() {}
	Image::SummaryData& image_data() { return image_data_; }
	const Image::SummaryData& image_data() const { return image_data_; }
	DICOM::SummaryInfo& dicom_info() { return dicom_info_; }
	DICOM::SummaryInfo dicom_info() const { return dicom_info_; }
	static bool write_image_data( DcmDataset*, const Image::DataSharedPtr&);

private:

	Image::SummaryData image_data_; // summary image data
	DICOM::SummaryInfo dicom_info_; // summary DICOM information
};

inline
File::File( const Image::SummaryData& image,
	const DICOM::SummaryInfo& dicom)
	:
	image_data_(image),
	dicom_info_(dicom)
{
}

} // namespace ScanAmati
