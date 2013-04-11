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
#include <gtkmm/button.h>
#include <gtkmm/checkbutton.h>

// files from src directory begin
#include "scanner/manager.hpp"

#include "global_strings.hpp"
// files from src directory end

#include "chip_capacities.hpp"
#include "lining_adjustment.hpp"

#include "utils.hpp"
#include "scanner_debug.hpp"

namespace ScanAmati {

namespace UI {

ScannerDebugDialog::ScannerDebugDialog( BaseObjectType* cobject,
	const Glib::RefPtr<Gtk::Builder>& builder)
	:
	ScannerTemplateDialog( cobject, builder, "scanner-debug"),
	button_altera_reset_(0),
	button_array_reset_(0),
	button_write_lining_(0),
	button_adjust_lining_(0),
	button_select_capacity_(0),
	button_turnoff_peltier_(0),
	checkbutton_temperature_control_(0)
{
	init_ui();

	connect_signals();

	show_all_children();
}

ScannerDebugDialog::~ScannerDebugDialog()
{
}

void
ScannerDebugDialog::init_ui()
{
	builder_->get_widget( "button-altera-reset", button_altera_reset_);
	builder_->get_widget( "button-array-reset", button_array_reset_);
	builder_->get_widget( "button-lining-write", button_write_lining_);
	builder_->get_widget( "button-lining-adjustment", button_adjust_lining_);
	builder_->get_widget( "button-chip-capacities", button_select_capacity_);
	builder_->get_widget( "button-peltier-turnoff", button_turnoff_peltier_);
	builder_->get_widget( "checkbutton-temperature-control",
		checkbutton_temperature_control_);
}

void
ScannerDebugDialog::connect_signals()
{
	button_adjust_lining_->signal_clicked().connect(sigc::mem_fun(
		*this, &ScannerDebugDialog::on_adjust_lining));
	button_select_capacity_->signal_clicked().connect(sigc::mem_fun(
		*this, &ScannerDebugDialog::on_select_capacity));

	button_write_lining_->signal_clicked().connect(sigc::mem_fun(
		*this, &ScannerDebugDialog::on_write_lining));

	button_altera_reset_->signal_clicked().connect(sigc::bind( sigc::mem_fun(
		*this, &ScannerDebugDialog::on_scanner_command),
		Scanner::COMMAND_ALTERA_RESET));
	button_array_reset_->signal_clicked().connect(sigc::bind( sigc::mem_fun(
		*this, &ScannerDebugDialog::on_scanner_command),
		Scanner::COMMAND_ARRAY_RESET));
	button_turnoff_peltier_->signal_clicked().connect(sigc::bind( sigc::mem_fun(
		*this, &ScannerDebugDialog::on_scanner_command),
		Scanner::COMMAND_PELTIER_OFF));
}

void
ScannerDebugDialog::block_interface(bool block)
{
	switch (block) {
	case true:
		button_altera_reset_->set_sensitive(false);
		button_array_reset_->set_sensitive(false);
		button_write_lining_->set_sensitive(false);
		button_adjust_lining_->set_sensitive(false);
		button_select_capacity_->set_sensitive(false);
		button_turnoff_peltier_->set_sensitive(false);
		checkbutton_temperature_control_->set_sensitive(false);
		break;
	case false:
		button_altera_reset_->set_sensitive(true);
		button_array_reset_->set_sensitive(true);
		button_write_lining_->set_sensitive(true);
		button_adjust_lining_->set_sensitive(true);
		button_select_capacity_->set_sensitive(true);
		button_turnoff_peltier_->set_sensitive(true);
		checkbutton_temperature_control_->set_sensitive(true);
		break;
	default:
		break;
	}
}

void
ScannerDebugDialog::update_scanner_state(const Scanner::State& state)
{
	if (!ui_state_initiated_) {
		set_temperature_control(state);
		ui_state_initiated_ = true;
	}

	Scanner::ManagerState mstate = state.manager_state();

	switch (mstate.run()) {
	case RUN_BACKGROUND:
		block_interface(false);
		break;
	default:
		block_interface();
		break;
	}
	signal_scanner_state_changed_(state);
}

void
ScannerDebugDialog::on_response(int)
{
	hide();
}

void
ScannerDebugDialog::on_scanner_command(Scanner::CommandType command)
{
	Scanner::SharedManager manager = Scanner::Manager::instance();

	switch (command) {
	case Scanner::COMMAND_PELTIER_OFF:
		if (checkbutton_temperature_control_->get_active())
			checkbutton_temperature_control_->set_active(false);
		break;
	default:
		break;
	}

	Scanner::Command* com = Scanner::Commands::create(command);
	Scanner::AcquisitionParameters params(com);
	manager->run( RUN_COMMANDS, params);
	block_interface(true);
}

void
ScannerDebugDialog::set_temperature_control(const Scanner::State& state)
{
	checkbutton_temperature_control_->set_active(state.temperature_control());

	checkbutton_temperature_control_->signal_toggled().connect(sigc::mem_fun(
		*this, &ScannerDebugDialog::on_temperature_control));
}

void
ScannerDebugDialog::on_temperature_control()
{
	bool state = checkbutton_temperature_control_->get_active();

	Scanner::SharedManager manager = Scanner::Manager::instance();
	if (!manager->set_temperature_control(state)) {
		WarningDialog dialog( *this, _("Warning"));
		dialog.set_message(_("Unable to change temperature control state."));
		dialog.set_secondary_text(_("Please, try later."));
		dialog.run();
	}
}

void
ScannerDebugDialog::on_select_capacity()
{
	ChipCapacitiesDialog* dialog = ChipCapacitiesDialog::create();
	if (dialog) {
		signal_scanner_state_changed_.connect(sigc::mem_fun(
			*dialog, &ChipCapacitiesDialog::update_scanner_state));
		dialog->run();
		delete dialog;
	}
}

void
ScannerDebugDialog::on_write_lining()
{
	block_interface(true);
	signal_write_lining_();
}

void
ScannerDebugDialog::on_adjust_lining()
{
	LiningAdjustmentDialog* dialog = LiningAdjustmentDialog::create();
	if (dialog) {
		signal_scanner_state_changed_.connect(sigc::mem_fun(
			*dialog, &LiningAdjustmentDialog::update_scanner_state));
		dialog->run();
		delete dialog;
	}
}

sigc::signal<void>
ScannerDebugDialog::signal_write_lining()
{
	return signal_write_lining_;
}

ScannerDebugDialog*
ScannerDebugDialog::create()
{
	ScannerDebugDialog* dialog = 0;
	std::string filename(builder_scanner_debug_dialog_filename);

	Glib::RefPtr<Gtk::Builder> builder = create_from_file(filename);

	if (builder)
		//Get the GtkBuilder-instantiated dialog
		builder->get_widget_derived( "dialog-window", dialog);

	return dialog;
}

} // namespace UI

} // namespace ScanAmati
