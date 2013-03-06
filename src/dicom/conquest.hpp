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
#include <glibmm/keyfile.h>
#include <glibmm/ustring.h>

namespace ScanAmati {

namespace DICOM {

class ConquestSettings {
public:
	ConquestSettings() {};
	ConquestSettings(const std::string& file);
	ConquestSettings(const ConquestSettings& foo);
	ConquestSettings& operator=(const ConquestSettings& foo);

	std::string sql_host() const { return sql_host_; }
	std::string sql_server() const { return sql_server_; }
	std::string username() const { return username_; }
	std::string password() const { return password_; }
	Glib::ustring get_pgsql_conninfo() const;
	Glib::ustring title() const;
	int port() const;

	std::string table_patients() const { return table_patients_; }
	std::string table_studies() const { return table_studies_; }
	std::string table_series() const { return table_series_; }
	std::string table_images() const { return table_images_; }
	std::string archive_directory() const { return archive_directory_; }
	Glib::ustring archive_directory(const Glib::ustring& path) const;

private:
	void load_initial_data();

	std::string sql_host_;
	std::string sql_server_;
	std::string username_;
	std::string password_;

	std::string table_patients_;
	std::string table_studies_;
	std::string table_series_;
	std::string table_images_;
	std::string archive_directory_;

	Glib::KeyFile keyfile_;
};

inline
Glib::ustring
ConquestSettings::get_pgsql_conninfo() const
{
	return Glib::ustring::compose( "dbname = '%1' user = '%2'", sql_server(),
		username());
}

class ConquestFiles {
public:
	ConquestFiles(const std::string& dir_name);

	std::string directory() { return directory_; }
	std::string settings_file() const;
	std::string log_file() const;
	std::string error_file() const;
	std::string executive_file() const;
	ConquestSettings get_settings() const;
	bool change_directory(const std::string& dir_name);
	bool check_files( bool& settings, bool& log, bool& error, bool& exec) const;

private:
	std::string directory_;
	std::string settings_file_;
	std::string log_file_;
	std::string error_log_file_;
	std::string executive_file_;
};

inline
ConquestFiles::ConquestFiles(const std::string& dir_name)
	:
	directory_(dir_name),
	settings_file_("dicom.ini"),
	log_file_("serverstatus.log"),
	error_log_file_("PacsTrouble.log"),
	executive_file_("dgate")
{
}

inline
std::string
ConquestFiles::settings_file() const
{
	return (directory_ + G_DIR_SEPARATOR_S + settings_file_); 
}

inline
std::string
ConquestFiles::log_file() const
{
	return (directory_ + G_DIR_SEPARATOR_S + log_file_);
}

inline
std::string
ConquestFiles::error_file() const
{
	return (directory_ + G_DIR_SEPARATOR_S + error_log_file_);
}

inline
std::string
ConquestFiles::executive_file() const
{
	return (directory_ + G_DIR_SEPARATOR_S + executive_file_);
}

} // namespace DICOM

} // namespace ScanAmati
