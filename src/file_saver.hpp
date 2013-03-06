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

#include <glibmm/fileutils.h>
#include <gdkmm/pixbuf.h>
#include <string>

#include "image/summary_data.hpp"
#include "exceptions.hpp"

namespace Gtk {
class FileFilter;
}

namespace ScanAmati {

namespace DICOM {
class SummaryInfo;
} // namespace DICOM

/** \brief Save DICOM file.
 * Saves image data and file information into file.
 * \param filename Name of file.
 * \param data     Scanner image data.
 * \param w        Image width.
 * \param h        Image height.
 * \return true in success false otherwise
 * \throw FileError exception
 */
class FileSaver {

public:
	FileSaver( const std::string& filename, const Gtk::FileFilter* filter);
	virtual ~FileSaver();
	bool save( const Image::DataSharedPtr& image,
		const Glib::RefPtr<Gdk::Pixbuf>& pixbuf,
		DICOM::SummaryInfo& info) throw(Exception);

	sigc::signal< void, const std::string&> signal_file_saved();
private:
	bool save_file( const Image::DataSharedPtr&,
		const Glib::RefPtr<Gdk::Pixbuf>& pixbuf,
		DICOM::SummaryInfo&) throw(Exception);

	bool save_file_dcm( const Image::DataSharedPtr&,
		DICOM::SummaryInfo& info) throw(Exception);
	bool save_file_jpg( const Glib::RefPtr<Gdk::Pixbuf>& pixbuf,
		DICOM::SummaryInfo& info) throw(Exception);
	bool save_file_png( const Glib::RefPtr<Gdk::Pixbuf>& pixbuf,
		DICOM::SummaryInfo& info) throw(Exception);
	bool save_file_pixbuf( const Glib::ustring& type,
		const Glib::RefPtr<Gdk::Pixbuf>& pixbuf,
		DICOM::SummaryInfo& info) throw(Exception);
	bool save_file_raw(const Image::DataSharedPtr&) throw(Exception);

	std::string filename_;
	const Gtk::FileFilter* filter_;

	sigc::signal< void, const std::string&> signal_file_saved_;
};

} // namespace ScanAmati
