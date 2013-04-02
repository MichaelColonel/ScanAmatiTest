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

#include <gtkmm/filefilter.h>
#include <glibmm/i18n.h>

#include "global_strings.hpp"
#include "file.hpp"
#include "file_loader.hpp"

namespace ScanAmati {

FileLoader::FileLoader( const std::string& filename,
	const Gtk::FileFilter* filter)
	:
	filename_(filename),
	filter_(filter)
{
}

FileLoader::~FileLoader()
{
}

void
FileLoader::load(FileLoadType type) throw(Exception)
{
	File file;
	bool res;
	try {
		if (filter_) {
			Glib::ustring name = filter_->get_name();

			if (!name.compare(gettext(filter_dcm_name)))
				res = load_dcm( file, type); // DICOM file
			else if (!name.compare(gettext(filter_raw_name)))
				res = load_raw(file); // Studio raw file
			else
				;
		}
		else {
			res = load_dcm( file, type); // DICOM file
		}
	}
	catch (const Exception& ex) {
		throw;
	}

	if (res)
		signal_file_loaded_( filename_, file);
}

Image::SummaryData
FileLoader::create_image_summary(const Image::DataSharedPtr& image)
{
	Image::SummaryData summary;
	summary.raw_data() = image;
	summary.fill_image_buffer();
	return summary;
}

bool
FileLoader::load_raw(File& file) throw(Exception)
{
	Image::DataSharedPtr image;

	std::ifstream filestream(filename_.c_str());

	if (filestream.is_open()) {
		filestream >> image;
		Image::SummaryData summary = create_image_summary(image);
		file = File(summary);
		return true;
	}
	else
		return false;
}

bool
FileLoader::load_dcm( File& file, FileLoadType) throw(Exception)
{
	Image::DataSharedPtr image;
	OFCondition result;

	DcmFileFormat fileformat;

	result = fileformat.loadFile(filename_.c_str());
	if (result.bad()) {
		throw Exception(result.text());
	}

	DcmDataset* dataset = fileformat.getDataset();
	if (dataset) {
		Uint16 h, w;
		result = dataset->findAndGetUint16( DCM_Rows, h); // height
		if (result.bad())
			throw Exception(_("Unable to load image height."));

		result = dataset->findAndGetUint16( DCM_Columns, w); // width
		if (result.bad())
			throw Exception(_("Unable to load image width."));
			
		const Uint16* pix;
		result = dataset->findAndGetUint16Array( DCM_PixelData, pix);
		if (result.good()) {
			const gint16* data = reinterpret_cast<const gint16*>(pix);
			if (data)
				image = Image::Data::create_from_data( w, h, data);
		}
		else
			throw Exception(_("Unable to load image data."));

		if (image) {
			Image::SummaryData summary = create_image_summary(image);
			file = File( summary, DICOM::SummaryInfo(dataset));
			return true;
		}
	}

	return false;
}

sigc::signal< void, const std::string&, File&>
FileLoader::signal_file_loaded()
{
	return signal_file_loaded_;
}

} // namespace ScanAmati
