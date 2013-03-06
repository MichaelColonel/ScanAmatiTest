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

#include <fstream>
#include <iostream>
#include <glibmm/i18n.h>
#include <glibmm/fileutils.h>
#include <glibmm/keyfile.h>
#include <glibmm/iochannel.h>

#include "server.hpp"

namespace ScanAmati {

namespace DICOM {

Glib::ustring
Server::called_address() const
{
	return Glib::ustring::compose( "%1:%2", host, Glib::ustring::format(port));
}

ServersConfiguration::ServersConfiguration(const std::string& filename)
	:
	filename_(filename),
	file_exsists_(false)
{
	file_exsists_ = Glib::file_test( filename_, Glib::FILE_TEST_EXISTS);
}

bool
ServersConfiguration::load(ServersMap& servers)
{
	bool res = false;
	ServersMap map;

	Glib::KeyFile keyfile;

	try {
		res = keyfile.load_from_file( filename_,
			Glib::KEY_FILE_KEEP_TRANSLATIONS |
			Glib::KEY_FILE_KEEP_COMMENTS);
	}
	catch (const Glib::Error& err) {
		std::cerr << "Can't load data from file: " << filename_ << ". ";
		switch (err.code()) {
		case Glib::KeyFileError::UNKNOWN_ENCODING:
			// unknown encoding of the file
			std::cerr << err.what() << std::endl;
			break;
		case Glib::KeyFileError::NOT_FOUND:
			// file not found
			std::cerr << err.what() << std::endl;
			break;
		default:
			std::cerr << err.what() << std::endl;
			break;
		}
	}

	if (res) {
		std::vector<Glib::ustring> groups = keyfile.get_groups();

		for ( std::vector<Glib::ustring>::const_iterator it = groups.begin();
			it != groups.end(); ++it) {
	
			Server server;
			try {
				server.host = keyfile.get_locale_string( *it, "Host");
				server.title = keyfile.get_locale_string( *it, "AE Title");
				server.port = keyfile.get_integer( *it, "Port");
				map[*it] = server;
			}
			catch (const Glib::KeyFileError& er) {
				std::cerr << er.what() << std::endl;
				return false;
			}
		}
		servers = map;
		res = true;
	}

	return res;
}

bool
ServersConfiguration::save_servers(const ServersMap& servers)
{
	Glib::KeyFile keyfile;
	bool res = false;
	for ( ServersMap::const_iterator it = servers.begin(); it != servers.end();
		++it) {
		keyfile.set_string( it->first, "Host", it->second.host);
		keyfile.set_string( it->first, "AE Title", it->second.title);
		keyfile.set_integer( it->first, "Port", it->second.port);
	}

	try {
		Glib::RefPtr<Glib::IOChannel> outfile =
			Glib::IOChannel::create_from_file( filename_, "w");
		outfile->write(keyfile.to_data());
		outfile->flush();
		res = true;
	}
	catch (const Glib::Error& error) {
		std::cerr << _("Failed to save dicom servers file.") << std::endl;
		std::cerr << _("An error occurred while attempting to save the file ")
			+ filename_ + _(", this file stores your dicom servers.");
		std::cerr << std::endl;
		std::cerr << _("The system reported this error as: ") + error.what();
		std::cerr << std::endl;
	}
	return res;
}

} // namespace DICOM

} // namespace ScanAmati
