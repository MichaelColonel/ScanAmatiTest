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

#include "dicom/server.hpp"

#include "dcmtk_defines.hpp"

#include "preferences.hpp"

namespace ScanAmati {

namespace UI {

class MainWindow;

} // namespace UI

class Application {

public:
	Application();
	virtual ~Application();
	void init();
	void finish();

private:
	static void static_init();
	static void static_finish();
	static int count_;
};

static Application application;

// Global Variables
extern struct GlobalApplicationVariables {
	GlobalApplicationVariables();
	bool debug;
	bool extend;
	OFLogger log;
	UI::MainWindow* main_window;
	Preferences prefs;
	DICOM::ServersMap dicom_servers;
	std::string ae_title;
	unsigned int port;
} app;

} // namespace ScanAmati
