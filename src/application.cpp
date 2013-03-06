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

#include <glibmm/thread.h>
#include <giomm/init.h>

#include "dicom/xmedcon_wrapper.h"

#include "global_strings.hpp"
#include "main_window.hpp"

#include "utils.hpp"

#include "application.hpp"

namespace ScanAmati {

int Application::count_ = 0;

struct GlobalApplicationVariables app;

Application::Application()
{
	if (count_++ == 0)
		static_init();
}

Application::~Application()
{
	if (--count_ == 0)
		static_finish();
}

void
Application::static_init()
{
	mdc_init();

	Gio::init();

	if (!Glib::thread_supported())
		Glib::thread_init();
}

void
Application::init()
{
	// load preferences
	if (!app.prefs.load())
		app.prefs.load_defaults();

	// load DICOM servers
	std::string file = get_dicom_servers_file();
	DICOM::ServersConfiguration config(file);
	if (config.load_servers(app.dicom_servers))
		OFLOG_DEBUG( app.log, "Dicom servers loaded successfully");	
}

void
Application::finish()
{
	// save preferences
	app.prefs.save();

	// save DICOM servers
	std::string file = get_dicom_servers_file();
	DICOM::ServersConfiguration config(file);
	if (config.save_servers(app.dicom_servers))
		OFLOG_DEBUG( app.log, "Dicom servers saved successfully");	
}

void
Application::static_finish()
{
	if (app.main_window) {
		delete app.main_window;
		app.main_window = 0;
	}

	mdc_finish();
}

GlobalApplicationVariables::GlobalApplicationVariables()
	:
	debug(false),
	extend(false),
	log(OFLog::getLogger("dcmtk.apps.sample")),
	main_window(0),
	ae_title("SCANAMATI"),
	port(10004)
{
}

} // namespace ScanAmati
