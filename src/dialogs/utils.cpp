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

#include <iostream>

// files from src directory begin
#include "application.hpp"
// files from src directory end

#include "utils.hpp"

namespace ScanAmati {

namespace UI {

Glib::RefPtr<Gtk::Builder>
create_from_file(const std::string& filename)
{
	Glib::RefPtr<Gtk::Builder> builder = Gtk::Builder::create();
	bool res;
	try {
		res = builder->add_from_file(filename);
	}
	catch (const Glib::FileError& ex) {
		std::cerr << "FileError: " << ex.what() << std::endl;
	}
	catch (const Glib::MarkupError& ex) {
		std::cerr << "MarkupError: " << ex.what() << std::endl;
	}
	catch (const Gtk::BuilderError& ex) {
		std::cerr << "BuilderError: " << ex.what() << std::endl;
	}
	catch (...) {
		std::cerr << "Unknown exception while loading";
		std::cerr << "\"" << filename << "\"" << std::endl;
	}

	if (!res) {
		OFLOG_DEBUG( app.log, "File load error.");
		builder.reset();
	}

	return builder;
}

} // namespace UI

} // namespace ScanAmati
