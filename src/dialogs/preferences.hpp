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
#include <gtkmm/liststore.h>
#include <gtkmm/treeselection.h>
#include <gtkmm/builder.h>

#include "dicom/server.hpp"

namespace Gtk {
class Entry;
class Button;
class SpinButton;
class TreeView;
} // namespace

namespace ScanAmati {

namespace UI {

class PreferencesDialog : public Gtk::Dialog {
public:
	static PreferencesDialog* create();
	PreferencesDialog( BaseObjectType* cobject,
		const Glib::RefPtr<Gtk::Builder>& builder);
	virtual ~PreferencesDialog();

protected:
	// UI methods:
	void init_ui();
	void connect_signals();

	// Handlers
	virtual void on_response(int);
	void on_init();
	void on_conquest_directory_clicked();
	void on_dicom_servers_double_clicked(GdkEventButton*);
	void on_dicom_servers_key_pressed(GdkEventKey*);
	void on_test_network_connection();
	void on_add_dicom_server();
	void on_remove_dicom_server();
	void on_edit_dicom_server();
	void add_dicom_server(const DICOM::Server&);
	void update_dicom_servers_treeview();

	// Members:
	Glib::RefPtr<Gtk::Builder> builder_;
	DICOM::ServersMap dicom_servers_;

	// Widgets:
	Gtk::Entry* entry_dicom_ae_title_;
	Gtk::SpinButton* spinbutton_dicom_port_;
	Gtk::Entry* entry_conquest_directory_;
	Gtk::Button* button_conquest_directory_;
	Gtk::Button* button_test_network_connection_;
	Gtk::Button* button_dicom_server_add_;
	Gtk::Button* button_dicom_server_remove_;
	Gtk::Button* button_dicom_server_edit_;
	Gtk::TreeView* treeview_dicom_servers_;
	Glib::RefPtr<Gtk::ListStore> liststore_dicom_servers_;
	Glib::RefPtr<Gtk::TreeSelection> selection_dicom_servers_;

	const struct DicomServersModelColumns : public Gtk::TreeModel::ColumnRecord
	{
		DicomServersModelColumns()
		{
			add(name);
			add(host);
			add(title);
			add(port);
		}

		Gtk::TreeModelColumn<Glib::ustring> name;
		Gtk::TreeModelColumn<Glib::ustring> host;
		Gtk::TreeModelColumn<Glib::ustring> title;
		Gtk::TreeModelColumn<guint> port;
	} dicom_servers_model_columns;

};

} // namespace UI

} // namespace ScanAmati
