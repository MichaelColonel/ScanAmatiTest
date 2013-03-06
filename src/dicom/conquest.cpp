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

#include <glibmm/fileutils.h>
#include <glibmm/ustring.h>
#include <list>
#include <map>
#include <iostream>

#include "conquest.hpp"

namespace {

const Glib::ustring default_group("sscscp");

enum FileType {
	FILE_INI, // "dicom.ini"
	FILE_LOG, // "serverstatus.log"
	FILE_ERR, // "PacsTrouble.log"
	FILE_EXE  // "dgate"
};

std::map< FileType, std::string>
file_map()
{
	std::map< FileType, std::string> map;

	map.insert(std::make_pair( FILE_INI, "dicom.ini"));
	map.insert(std::make_pair( FILE_LOG, "serverstatus.log"));
	map.insert(std::make_pair( FILE_ERR, "PacsTrouble.log"));
	map.insert(std::make_pair( FILE_EXE, "dgate"));

	return map;
}

} // namespace

namespace ScanAmati {

namespace DICOM {

bool
ConquestFiles::change_directory(const std::string& dirname)
{
	if (Glib::file_test( dirname, Glib::FILE_TEST_IS_DIR)) {
		directory_ = dirname;
		return true;
	}
	else
		return false;
}
	
bool
ConquestFiles::check_files( bool& settings, bool& log,
	bool& error, bool& exec) const
{
	settings = log = error = exec = false;
	if (!Glib::file_test( directory_, Glib::FILE_TEST_IS_DIR))
		return false;

	Glib::Dir dir(directory_);
	std::list<std::string> entries( dir.begin(), dir.end());

	std::map< FileType, std::string> files = file_map();

	std::list<std::string>::const_iterator iter;
	for ( iter = entries.begin(); iter != entries.end(); ++iter) {

		std::map< FileType, std::string>::const_iterator it;
		for ( it = files.begin(); it != files.end(); ++it) {

			if (it->second == *iter) {
				switch (it->first) {
				case FILE_INI:
					settings = true;
					break;
				case FILE_LOG:
					log = true;
					break;
				case FILE_ERR:
					error = true;
					break;
				case FILE_EXE:
					exec = true;
					break;
				default:
					break;
				}
			}
		}
	}
	return (settings && log && error && exec);
}

ConquestSettings
ConquestFiles::get_settings() const
{
	bool settings, log, error, exec, res;

	res = check_files( settings, log, error, exec);

	if (res && settings)
		return ConquestSettings(settings_file());
	else
		return ConquestSettings();
}

ConquestSettings::ConquestSettings(const std::string& file)
	:
	sql_host_("localhost"),
	sql_server_("conquest"),
	username_("postgres"),
	password_("postgres"),
	table_patients_("DICOMPatients"),
	table_studies_("DICOMStudies"),
	table_series_("DICOMSeries"),
	table_images_("DICOMImages"),
	archive_directory_("MAGDevice0")
{
	if (!Glib::file_test( file, Glib::FILE_TEST_EXISTS)) {
		std::cerr << "ini file doesn't exist" << std::endl;
		std::cerr << "program will use default settings" << std::endl;
	}
	else {
		Glib::KeyFileFlags flag =
			Glib::KEY_FILE_KEEP_TRANSLATIONS | Glib::KEY_FILE_KEEP_COMMENTS;
		try {
			keyfile_.load_from_file( file, flag);
			load_initial_data();
		}
		catch (const Glib::KeyFileError& err) {
			switch (err.code()) {
			case Glib::KeyFileError::UNKNOWN_ENCODING:
				std::cerr << "unknown encoding of ini file" << std::endl;
				break;
			case Glib::KeyFileError::NOT_FOUND:
				std::cerr << "ini file not found" << std::endl;
				break;
			default:
				std::cerr << "error while loading ini file" << std::endl;
				break;
			}
		}
	}
}

void
ConquestSettings::load_initial_data()
{
	if (keyfile_.has_key( default_group, "SQLHost"))
		sql_host_ = keyfile_.get_string( default_group, "SQLHost").raw();
	if (keyfile_.has_key( default_group, "SQLServer"))
		sql_server_ = keyfile_.get_string( default_group, "SQLServer").raw();
	if (keyfile_.has_key( default_group, "Username"))
		username_ = keyfile_.get_string( default_group, "Username").raw();
	if (keyfile_.has_key( default_group, "Password"))
		password_ = keyfile_.get_string( default_group, "Password").raw();
	if (keyfile_.has_key( default_group, "PatientTableName"))
		table_patients_ = keyfile_.get_string( default_group,
			"PatientTableName").raw();
	if (keyfile_.has_key( default_group, "StudyTableName"))
		table_studies_ = keyfile_.get_string( default_group,
			"StudyTableName").raw();
	if (keyfile_.has_key( default_group, "SeriesTableName"))
		table_series_ = keyfile_.get_string( default_group,
			"SeriesTableName").raw();
	if (keyfile_.has_key( default_group, "ImageTableName"))
		table_images_ = keyfile_.get_string( default_group,
			"SeriesTableName").raw();
	if (keyfile_.has_key( default_group, "MAGDevice0"))
		archive_directory_ = keyfile_.get_string( default_group,
			"MAGDevice0").raw();
}

Glib::ustring
ConquestSettings::archive_directory(const Glib::ustring& key) const
{
	return (keyfile_.has_key( default_group, key)) ?
		keyfile_.get_string( default_group, key) : Glib::ustring();
}

Glib::ustring
ConquestSettings::title() const
{
	return keyfile_.get_string( default_group, "MyACRNema");
}

int
ConquestSettings::port() const
{
	return keyfile_.get_integer( default_group, "TCPPort");
}

} // namespace DICOM

} // namespace ScanAmati
