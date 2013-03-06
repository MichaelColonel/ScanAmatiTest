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
#include <gtkmm/liststore.h>

#include <giomm/filemonitor.h>
#include <giomm/fileinputstream.h>

struct T_DIMSE_StoreProgress;
struct T_DIMSE_C_StoreRQ;

namespace Gtk {
class TextView;
class ComboBox;
class ProgressBar;
class Button;
} // namespace Gtk

namespace ScanAmati {

namespace DICOM {
class Dataset;
} // namespace DICOM;

namespace UI {

class StoreDicomDialog : public Gtk::Dialog {
public:
	static StoreDicomDialog* create(DICOM::Dataset*);
	StoreDicomDialog( BaseObjectType* cobject,
		const Glib::RefPtr<Gtk::Builder>& builder);
	virtual ~StoreDicomDialog();
	virtual void store_progress( void*, T_DIMSE_StoreProgress*,
		T_DIMSE_C_StoreRQ*);

protected:
	// UI methods:
	void init_ui();
	void connect_signals();
	void load_preferences();
	void save_preferences();
	void fill_servers_liststore();

	// Signal handlers:
	virtual void on_response(int);
	void on_dicom_server_changed();
	void on_store();

	// Conquest status log file handler
	void on_status_changed( const Glib::RefPtr<Gio::File>&,
		const Glib::RefPtr<Gio::File>&, Gio::FileMonitorEvent);
	// Conquest errors log file handler
	void on_errors_changed( const Glib::RefPtr<Gio::File>&,
		const Glib::RefPtr<Gio::File>&, Gio::FileMonitorEvent);

	// Members:
	Glib::RefPtr<Gtk::Builder> builder_;
	Gtk::ProgressBar* progressbar_;
	Gtk::Button* button_store_;
	Gtk::ComboBox* combobox_servers_;
	Glib::RefPtr<Gtk::ListStore> liststore_servers_;

	// Conquest localhost server members
	Gtk::TextView* textview_status_;
	Gtk::TextView* textview_errors_;

	Glib::RefPtr<Gio::FileMonitor> filemonitor_status_;
	Glib::RefPtr<Gio::FileInputStream> filestream_status_;
	Glib::RefPtr<Gio::FileMonitor> filemonitor_errors_;
	Glib::RefPtr<Gio::FileInputStream> filestream_errors_;

	goffset offset_status_;
	goffset offset_errors_;

	DICOM::Dataset* dataset_;
};

} // namespace UI

} // namespace ScanAmati
