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
#include <glibmm/regex.h>

#include "dicom/summary_information.hpp"

#include <dcmtk/dcmdata/dctk.h>

#include "global_strings.hpp"
#include "application.hpp"
#include "file_saver.hpp"

namespace ScanAmati {

FileSaver::FileSaver(const std::string& filename,
	const Gtk::FileFilter* filter)
	:
	filename_(filename),
	filter_(filter)
{
	bool res = false;
	Glib::ustring name = filter_->get_name();
	std::string key = name.collate_key();

	const char* suffix = 0;
	if (!key.compare(Glib::ustring(gettext(filter_dcm_name)).collate_key()))
		suffix = ".dcm";
	else if (!key.compare(Glib::ustring(gettext(filter_raw_name)).collate_key())) {
		if (app.extend)
			suffix = ".raw";
	}
	else if (!key.compare(Glib::ustring(gettext(filter_jpg_name)).collate_key()))
		suffix = ".jpg";
	else if (!key.compare(Glib::ustring(gettext(filter_png_name)).collate_key()))
		suffix = ".png";
	else
		;

	std::list< std::string > parts = Glib::Regex::split_simple(
		dot_pattern, filename);
	if (parts.size() >= 1) {
		filename_ = *parts.begin() + suffix;
	}
	else {
	}
}

FileSaver::~FileSaver()
{
}

bool
FileSaver::save( const Image::DataSharedPtr& image,
	const Glib::RefPtr<Gdk::Pixbuf>& pixbuf,
	DICOM::SummaryInfo& info) throw(Exception)
{
	if (!image || image->empty())
		throw Exception(_("Image data is empty."));

	bool res = false;

	try {
		res = save_file( image, pixbuf, info);
		if (res)
			signal_file_saved_(filename_);
	}
	catch (const Exception&) {
		throw;
	}

	return res;
}

bool
FileSaver::save_file( const Image::DataSharedPtr& image,
	const Glib::RefPtr<Gdk::Pixbuf>& pixbuf,
	DICOM::SummaryInfo& info) throw(Exception)
{
	bool res = false;
	Glib::ustring name = filter_->get_name();

	if (!name.compare(gettext(filter_dcm_name)))
		res = save_file_dcm( image, info); // DICOM file
	else if (!name.compare(gettext(filter_raw_name)))
		res = save_file_raw(image); // Studio raw file
	else if (!name.compare(gettext(filter_jpg_name)))
		res = save_file_jpg( pixbuf, info); // JPEG file
	else if (!name.compare(gettext(filter_png_name)))
		res = save_file_png( pixbuf, info); // PNG file
	else
		;

	return res;
}

bool
FileSaver::save_file_dcm( const Image::DataSharedPtr& image,
	DICOM::SummaryInfo& info) throw(Exception)
{
	DcmFileFormat fileformat;
	DcmMetaInfo* metainfo = fileformat.getMetaInfo();
	DcmDataset* dataset = fileformat.getDataset();

	metainfo->putAndInsertString( DCM_MediaStorageSOPClassUID,
		UID_DigitalXRayImageStorageForPresentation);

	info.save(dataset);

	Uint16* data = reinterpret_cast<Uint16*>(image->data());

	dataset->putAndInsertUint16( DCM_Rows, image->height());
	dataset->putAndInsertUint16( DCM_Columns, image->width());

	if (data) {
		dataset->putAndInsertUint16Array( DCM_PixelData, data,
			image->width() * image->height());
		OFCondition status = fileformat.saveFile( filename_.c_str(),
			EXS_LittleEndianExplicit, EET_UndefinedLength, EGL_withGL);
		if (!status.good())
			return false;
	}

	return true;
}

bool
FileSaver::save_file_pixbuf( const Glib::ustring& image_type,
	const Glib::RefPtr<Gdk::Pixbuf>& pixbuf,
	DICOM::SummaryInfo&) throw(Exception)
{
	try {
		pixbuf->save( filename_, image_type);
	}
	catch (const Glib::FileError& err) {
		throw Exception(err.what());
	}
	catch (const Gdk::PixbufError& err) {
		throw Exception(err.what());
	}

	return true;
}

bool
FileSaver::save_file_jpg( const Glib::RefPtr<Gdk::Pixbuf>& pixbuf,
	DICOM::SummaryInfo& info) throw(Exception)
{
	try {
		save_file_pixbuf( "jpeg", pixbuf, info);
	}
	catch (const Exception&) {
		throw;
	}

	return true;
}

bool
FileSaver::save_file_png( const Glib::RefPtr<Gdk::Pixbuf>& pixbuf,
	DICOM::SummaryInfo& info) throw(Exception)
{
	try {
		save_file_pixbuf( "png", pixbuf, info);
	}
	catch (const Exception&) {
		throw;
	}

	return true;
}

bool
FileSaver::save_file_raw(const Image::DataSharedPtr& image) throw(Exception)
{
	std::ofstream file(filename_.c_str());
	std::cout << "FILE SAVED" << std::endl;
	file << image;
	file.close();

	return true;
}

sigc::signal< void, const std::string&>
FileSaver::signal_file_saved()
{
	return signal_file_saved_;
}

} // namespace ScanAmati
