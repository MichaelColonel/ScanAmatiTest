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

#include <map>
#include <glibmm/ustring.h>

namespace ScanAmati {

namespace DICOM {

struct Server {
	Glib::ustring called_address() const;
	Glib::ustring name; // key in the ServersMap
	Glib::ustring host; // aka ip, peer, hostname
	Glib::ustring title; // Application Entity Title
	unsigned int port;
};

typedef std::map< Glib::ustring, Server> ServersMap;

class ServersConfiguration {
public:
	ServersConfiguration(const std::string& filename);
	bool load_servers(ServersMap&);
	bool save_servers(const ServersMap&);
private:
	bool load(ServersMap&);
	std::string filename_;
	bool file_exsists_;
};

inline
bool
ServersConfiguration::load_servers(ServersMap& servers)
{
	return file_exsists_ ? load(servers) : false;
}

} // namespace DICOM

} // namespace ScanAmati
