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

#include <glibmm/i18n.h>
#include <gtkmm/stock.h>
#include <gtkmm/image.h>
#include <gtkmm/entry.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/button.h>

// files from src directory begin
#include "dicom/server.hpp"
#include "global_strings.hpp"
// files from src directory begin

#include "utils.hpp"
#include "dicom_server.hpp"

namespace ScanAmati {

namespace UI {

DicomServer::DicomServer( BaseObjectType* cobject,
	const Glib::RefPtr<Gtk::Builder>& builder)
	:
	Gtk::Dialog(cobject),
	builder_(builder),
	entry_name_(0),
	entry_title_(0),
	entry_host_(0),
	spinbutton_port_(0),
	button_action_(0)
{
	init_ui();

	connect_signals();

	show_all_children();
}

DicomServer::~DicomServer()
{
}

void
DicomServer::init_ui()
{
	builder_->get_widget( "entry-name", entry_name_);
	builder_->get_widget( "entry-title", entry_title_);
	builder_->get_widget( "entry-host", entry_host_);
	builder_->get_widget( "spinbutton-port", spinbutton_port_);
	builder_->get_widget( "button-action", button_action_);
}

void
DicomServer::connect_signals()
{
}

void
DicomServer::set_server(const DICOM::Server& server)
{
	set_title(_("Edit Server Data"));

	Gtk::ButtonBox* box;
	builder_->get_widget( "dialog-action-area", box);
	box->remove(*button_action_);

	add_button( Gtk::Stock::EDIT, 1);

	entry_name_->set_text(server.name);
	entry_host_->set_text(server.host);
	entry_title_->set_text(server.title);
	spinbutton_port_->set_value(server.port);
}

void
DicomServer::on_response(int res)
{
	switch (res) {
	case 0:
		break;
	case 1:
		{
			DICOM::Server server;
			server.name = entry_name_->get_text();
			server.host = entry_host_->get_text();
			server.title = entry_title_->get_text();
			server.port = spinbutton_port_->get_value_as_int();

			signal_new_dicom_server_(server);
		}
		break;
	default:
		break;
	}

	hide();
}

sigc::signal< void, const DICOM::Server&>
DicomServer::signal_new_dicom_server()
{
	return signal_new_dicom_server_;
}

DicomServer*
DicomServer::create()
{
	DicomServer* dialog = 0;
	std::string filename(builder_dicom_server_dialog_filename);

	Glib::RefPtr<Gtk::Builder> builder = create_from_file(filename);

	if (builder)
		//Get the GtkBuilder-instantiated dialog
		builder->get_widget_derived( "dialog-window", dialog);

	return dialog;
}

} // namespace UI

} // namespace ScanAmati
