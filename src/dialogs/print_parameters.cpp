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

#include <gtkmm/messagedialog.h>

#include "utils.hpp"

// files from src directory begin
#include "global_strings.hpp"
// files from src directory end

#include "print_parameters.hpp"

namespace ScanAmati {

namespace UI {

PrintParametersDialog::PrintParametersDialog( BaseObjectType* cobject,
	const Glib::RefPtr<Gtk::Builder>& builder)
	:
	Gtk::Dialog(cobject),
	builder_(builder_)
{
	init_ui();

	connect_signals();

	show_all_children();
}

PrintParametersDialog::~PrintParametersDialog()
{
}

void
PrintParametersDialog::init_ui()
{
}

void
PrintParametersDialog::connect_signals()
{
}

void
PrintParametersDialog::on_printoperation_status_changed(
	const Glib::RefPtr<Print::Operation>& operation)
{
	Glib::ustring status_msg;

	if (operation->is_finished()) {
		status_msg = "Print job completed.";
	}
	else {
		//You could also use get_status().
		status_msg = operation->get_status_string();
	}
}

void
PrintParametersDialog::on_printoperation_done(
	Gtk::PrintOperationResult result,
	const Glib::RefPtr<Print::Operation>& operation)
{
	//Printing is "done" when the print data is spooled.

	if (result == Gtk::PRINT_OPERATION_RESULT_ERROR) {
		Gtk::MessageDialog dialog(*this, "Error printing form", false,
			Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
		dialog.run();
	}
	else if (result == Gtk::PRINT_OPERATION_RESULT_APPLY) {
		//Update PrintSettings with the ones used in this PrintOperation:
		print_settings_ = operation->get_print_settings();
	}

	if (!operation->is_finished()) {
		//We will connect to the status-changed signal to track status
		//and update a status bar. In addition, you can, for example,
		//keep a list of active print operations, or provide a progress dialog.
		operation->signal_status_changed().connect(sigc::bind( sigc::mem_fun(
			*this, &PrintParametersDialog::on_printoperation_status_changed),
			operation));
	}
}

void
PrintParametersDialog::on_response(int)
{
	hide();
}

PrintParametersDialog*
PrintParametersDialog::create()
{
	PrintParametersDialog* dialog = 0;
	std::string filename(builder_lining_adjustment_dialog_filename);

	Glib::RefPtr<Gtk::Builder> builder = create_from_file(filename);

	if (builder)
		//Get the GtkBuilder-instantiated dialog
		builder->get_widget_derived( "dialog-window", dialog);

	return dialog;
}

} // namespace UI

} // namespace ScanAmati
