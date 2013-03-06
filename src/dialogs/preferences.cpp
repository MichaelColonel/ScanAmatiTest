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

#include <gtkmm/stock.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/button.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/treeview.h>

#include <errno.h>

// files from src directory begin
#include "dicom/user_commands.hpp"

#include "application.hpp"
#include "exceptions.hpp"
#include "global_strings.hpp"
// files from src directory begin

#include "dicom_server.hpp"

#include "utils.hpp"
#include "preferences.hpp"

namespace ScanAmati {

namespace UI {

PreferencesDialog::PreferencesDialog( BaseObjectType* cobject,
	const Glib::RefPtr<Gtk::Builder>& builder)
	:
	Gtk::Dialog(cobject),
	builder_(builder),
	entry_dicom_ae_title_(0),
	spinbutton_dicom_port_(0),
	button_conquest_directory_(0),
	button_test_network_connection_(0),
	button_dicom_server_add_(0),
	button_dicom_server_remove_(0),
	button_dicom_server_edit_(0),
	treeview_dicom_servers_(0)
{
	init_ui();

	connect_signals();

	show_all_children();
}

PreferencesDialog::~PreferencesDialog()
{
}

void
PreferencesDialog::init_ui()
{
	builder_->get_widget( "entry-application-entity-title", entry_dicom_ae_title_);
	builder_->get_widget( "spinbutton-application-entity-port", spinbutton_dicom_port_);
	builder_->get_widget( "entry-conquest-directory", entry_conquest_directory_);
	builder_->get_widget( "button-conquest-directory", button_conquest_directory_);
	builder_->get_widget( "button-test-connection", button_test_network_connection_);
	builder_->get_widget( "button-add-server", button_dicom_server_add_);
	builder_->get_widget( "button-remove-server", button_dicom_server_remove_);
	builder_->get_widget( "button-edit-server", button_dicom_server_edit_);
	builder_->get_widget( "treeview-dicom-servers", treeview_dicom_servers_);

	// set ConQuest directory
	Glib::ustring dir = app.prefs.get<Glib::ustring>( "ConQuest",
		"localhost-server-directory");
	entry_conquest_directory_->set_text(dir);

	// create model
	liststore_dicom_servers_ = Gtk::ListStore::create(dicom_servers_model_columns);

	// create treeview
	treeview_dicom_servers_->set_model(liststore_dicom_servers_);

	selection_dicom_servers_ = treeview_dicom_servers_->get_selection();
	selection_dicom_servers_->set_mode(Gtk::SELECTION_SINGLE);

	// add model columns
	{
		Gtk::TreeView::Column* column = Gtk::manage(new Gtk::TreeView::Column(
			Q_("DICOM Server|Name"),
			dicom_servers_model_columns.name));
		column->set_alignment(Gtk::ALIGN_CENTER);
		column->set_expand(true);
		Gtk::CellRenderer* renderer = column->get_first_cell_renderer();
		renderer->property_xalign() = .5;
		treeview_dicom_servers_->append_column(*column);
	}

	{
		Gtk::TreeView::Column* column = Gtk::manage(new Gtk::TreeView::Column(
			_("IP / Host"),
			dicom_servers_model_columns.host));
		column->set_alignment(Gtk::ALIGN_CENTER);
		column->set_expand(true);
		Gtk::CellRenderer* renderer = column->get_first_cell_renderer();
		renderer->property_xalign() = .5;
		treeview_dicom_servers_->append_column(*column);
	}

	{
		Gtk::TreeView::Column* column = Gtk::manage(new Gtk::TreeView::Column(
			_("AE Title"),
			dicom_servers_model_columns.title));
		column->set_alignment(Gtk::ALIGN_CENTER);
		column->set_expand(true);
		Gtk::CellRenderer* renderer = column->get_first_cell_renderer();
		renderer->property_xalign() = .5;
		treeview_dicom_servers_->append_column(*column);
	}

	{
		Gtk::TreeView::Column* column = Gtk::manage(new Gtk::TreeView::Column(
			_("Port"),
			dicom_servers_model_columns.port));
		column->set_alignment(Gtk::ALIGN_CENTER);
		column->set_expand(true);
		Gtk::CellRenderer* renderer = column->get_first_cell_renderer();
		renderer->property_xalign() = .5;
		treeview_dicom_servers_->append_column(*column);
	}

	dicom_servers_ = app.dicom_servers;
	update_dicom_servers_treeview();
}

void
PreferencesDialog::connect_signals()
{
	button_conquest_directory_->signal_clicked().connect(sigc::mem_fun(
		*this, &PreferencesDialog::on_conquest_directory_clicked));

	button_test_network_connection_->signal_clicked().connect(sigc::mem_fun(
		*this, &PreferencesDialog::on_test_network_connection));
	button_dicom_server_add_->signal_clicked().connect(sigc::mem_fun(
		*this, &PreferencesDialog::on_add_dicom_server));
	button_dicom_server_remove_->signal_clicked().connect(sigc::mem_fun(
		*this, &PreferencesDialog::on_remove_dicom_server));
	button_dicom_server_edit_->signal_clicked().connect(sigc::mem_fun(
		*this, &PreferencesDialog::on_edit_dicom_server));

	treeview_dicom_servers_->signal_button_press_event().connect_notify(
		sigc::mem_fun( *this, &PreferencesDialog::on_dicom_servers_double_clicked));
	treeview_dicom_servers_->signal_key_press_event().connect_notify(
		sigc::mem_fun( *this, &PreferencesDialog::on_dicom_servers_key_pressed));
}

void
PreferencesDialog::add_dicom_server(const DICOM::Server& server)
{
	DICOM::Server new_server = server;
	new_server.name.clear();

	dicom_servers_[server.name] = new_server;
	update_dicom_servers_treeview();
}

void
PreferencesDialog::on_response(int res)
{
	switch (res) {
	case 0: // Close
		break;
	case 1: // Apply
		{
			app.ae_title = entry_dicom_ae_title_->get_text();
			app.port = spinbutton_dicom_port_->get_value_as_int();
			app.dicom_servers = dicom_servers_;
			Glib::ustring dir = entry_conquest_directory_->get_text();
			app.prefs.set( "ConQuest", "localhost-server-directory", dir);
		}
		break;
	default:
		break;
	}

	hide();
}

PreferencesDialog*
PreferencesDialog::create()
{
	PreferencesDialog* dialog = 0;
	std::string filename(builder_preferences_dialog_filename);

	Glib::RefPtr<Gtk::Builder> builder = create_from_file(filename);

	if (builder)
		//Get the GtkBuilder-instantiated dialog
		builder->get_widget_derived( "dialog-window", dialog);

	return dialog;
}

void
PreferencesDialog::on_conquest_directory_clicked()
{
	Gtk::FileChooserDialog dialog( *this, _("Please select a directory"),
		Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);

	// Add response buttons in the dialog:
	dialog.add_button( Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	dialog.add_button( Gtk::Stock::OK, Gtk::RESPONSE_OK);

	Glib::ustring folder = entry_conquest_directory_->get_text();
	if (!folder.empty())
		dialog.set_current_folder(folder);

	// Show the dialog and wait for a user response:
	int result = dialog.run();

	// Handle the response:
	switch (result) {
    case Gtk::RESPONSE_OK:
		{
			// Notice that this is a std::string, not a Glib::ustring.
			std::string filename = dialog.get_filename();
			Glib::ustring fname = Glib::filename_to_utf8(filename);
			entry_conquest_directory_->set_text(fname);
		}
		break;
    case Gtk::RESPONSE_CANCEL:
		break;
	default:
		break;
	}
}

void
PreferencesDialog::on_add_dicom_server()
{
	DicomServer* dialog = DicomServer::create();
	if (dialog) {
		dialog->signal_new_dicom_server().connect(sigc::mem_fun(
			*this, &PreferencesDialog::add_dicom_server));

		dialog->run();
		delete dialog;
	}
}

void
PreferencesDialog::on_dicom_servers_double_clicked(GdkEventButton* event)
{
	switch (event->type) {
	case GDK_2BUTTON_PRESS:
		on_edit_dicom_server();
		break;
	default:
		break;
	}
}

void
PreferencesDialog::on_dicom_servers_key_pressed(GdkEventKey* event)
{
	switch (event->type) {
	case GDK_KEY_PRESS:
		if (event->keyval == GDK_KEY_Delete)
			on_remove_dicom_server();
		break;
	default:
		break;
	}
}

void
PreferencesDialog::on_remove_dicom_server()
{
	Glib::RefPtr<Gtk::TreeModel> model =
		Glib::RefPtr<Gtk::TreeModel>::cast_dynamic(liststore_dicom_servers_);

	Gtk::TreeModel::iterator iter =
		selection_dicom_servers_->get_selected(model);

	if (iter) {
		Gtk::TreeModel::Row row = *iter;
		dicom_servers_.erase(row[dicom_servers_model_columns.name]);
		update_dicom_servers_treeview();
	}
}

void
PreferencesDialog::on_edit_dicom_server()
{
	Glib::RefPtr<Gtk::TreeModel> model =
		Glib::RefPtr<Gtk::TreeModel>::cast_dynamic(liststore_dicom_servers_);

	Gtk::TreeModel::iterator iter =
		selection_dicom_servers_->get_selected(model);

	if (iter) {
		Gtk::TreeModel::Row row = *iter;
		DICOM::Server server;
				
		server.name = row[dicom_servers_model_columns.name];
		server.host = row[dicom_servers_model_columns.host];
		server.title = row[dicom_servers_model_columns.title];
		server.port = row[dicom_servers_model_columns.port];

		DicomServer* dialog = DicomServer::create();
		if (dialog) {
			dialog->signal_new_dicom_server().connect(sigc::mem_fun(
				*this, &PreferencesDialog::add_dicom_server));

			dialog->set_server(server);

			dialog->run();
			delete dialog;
		}
	}
}

void
PreferencesDialog::on_test_network_connection()
{
	Glib::RefPtr<Gtk::TreeModel> model =
		Glib::RefPtr<Gtk::TreeModel>::cast_dynamic(liststore_dicom_servers_);

	Gtk::TreeModel::iterator iter =
		selection_dicom_servers_->get_selected(model);
	if (iter) {
		DICOM::Server server;
		Gtk::TreeModel::Row row = *iter;

		server.host = row[dicom_servers_model_columns.host];
		server.title = row[dicom_servers_model_columns.title];
		server.port = row[dicom_servers_model_columns.port];

		try {
			DICOM::EchoCommand echo(server);
			bool res = echo.run();
			if (res) {
				InfoDialog dialog( *this, _("<b>Connection successful!</b>"));
				dialog.set_title(Q_("Connection|State"));
				dialog.run();
			}
		}
		catch (const Exception& ex) {
			Glib::ustring msg;
			switch (errno) {
			case ECONNREFUSED:
				msg = Glib::ustring(_("Connection refused by host."));
				break;
			default:
				msg = ex.what();
				break;
			}

			ErrorDialog dialog( *this, _("<b>Connection failed!</b>"));
			dialog.set_title(Q_("Connection|State"));
			dialog.set_secondary_text(msg);
			dialog.run();
		}
	}
	else {
		WarningDialog dialog( *this, _("<b>Please select DICOM server!</b>"));
		dialog.run();
	}
}

void
PreferencesDialog::update_dicom_servers_treeview()
{
	liststore_dicom_servers_->clear();

	for ( DICOM::ServersMap::const_iterator it = dicom_servers_.begin();
		it != dicom_servers_.end(); ++it) {
		Gtk::TreeRow row = *(liststore_dicom_servers_->append());

		row[dicom_servers_model_columns.name] = it->first;
		row[dicom_servers_model_columns.host] = it->second.host;
		row[dicom_servers_model_columns.title] = it->second.title;
		row[dicom_servers_model_columns.port] = it->second.port;
	}

	bool state = dicom_servers_.empty();

	button_test_network_connection_->set_sensitive(!state);
	button_dicom_server_remove_->set_sensitive(!state);
	button_dicom_server_edit_->set_sensitive(!state);
}

} // namespace UI

} // namespace ScanAmati
