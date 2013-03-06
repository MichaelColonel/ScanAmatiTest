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

#include <string>

#include "image/summary_data.hpp"
#include "dicom/summary_information.hpp"

#include "exceptions.hpp"

namespace Gtk {
class FileFilter;
}

namespace ScanAmati {

struct File;

enum FileLoadType {
	LOAD_INFO, // Load only dicom summary information
	LOAD_ALL // load everything
};

class FileLoader {

public:
	FileLoader( const std::string& filename,
		const Gtk::FileFilter* filter = 0);
	virtual ~FileLoader();

	/** \brief Load DICOM file. 
	 * 
	 * Loads image and study data from a DICOM file.
	 * 
	 * \param fileinfo       Structure which stores DICOM data.
	 * \return true in success false otherwise
	 * \throw FileError exception
	 */
	void load(FileLoadType type = LOAD_ALL) throw(Exception);
	sigc::signal< void, const std::string&, File&> signal_file_loaded();

private:
	bool load_dcm( File& file, FileLoadType) throw(Exception);
	bool load_raw(File& file) throw(Exception);
	Image::SummaryData create_image_summary(const Image::DataSharedPtr&);

	sigc::signal< void, const std::string&, File&> signal_file_loaded_;

	std::string filename_;
	const Gtk::FileFilter* filter_;
};

} // namespace ScanAmati
