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

#include <gtkmm/dialog.h>
#include <gtkmm/builder.h>

namespace Gtk {
class Entry;
class Button;
class SpinButton;
}

namespace ScanAmati {

namespace DICOM {
struct Server;
}

namespace UI {

class DicomServer : public Gtk::Dialog {
public:
	static DicomServer* create();
	DicomServer( BaseObjectType* cobject,
		const Glib::RefPtr<Gtk::Builder>& builder);
	virtual ~DicomServer();
	void set_server(const DICOM::Server&);
	sigc::signal< void, const DICOM::Server&> signal_new_dicom_server();

protected:
	// UI methods:
	void init_ui();
	void connect_signals();
	virtual void on_response(int);

	// UI Widgets:
	Glib::RefPtr<Gtk::Builder> builder_;
	Gtk::Entry* entry_name_;
	Gtk::Entry* entry_title_;
	Gtk::Entry* entry_host_;
	Gtk::SpinButton* spinbutton_port_;
	Gtk::Button* button_action_;

	// signals:
	sigc::signal< void, const DICOM::Server&> signal_new_dicom_server_;
};

} // namespace UI

} // namespace ScanAmati
