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

#include <gtkmm/scale.h>
#include <gtkmm/spinbutton.h>

// files from src directory begin
#include "application.hpp"
#include "scanner/manager.hpp"
#include "widgets/utils.hpp"

#include "global_strings.hpp"
// files from src directory end

#include "utils.hpp"

#include "temperature_margins.hpp"

namespace ScanAmati {

namespace UI {

TemperatureMarginsDialog::TemperatureMarginsDialog( BaseObjectType* cobject,
	const Glib::RefPtr<Gtk::Builder>& builder)
	:
	ScannerTemplateDialog( cobject, builder, "temperature-margins"),
	hscale_temperature_average_value_(0),
	spinbutton_temperature_spread_value_(0)
{
	init_ui();

	connect_signals();

	show_all_children();
}

TemperatureMarginsDialog::~TemperatureMarginsDialog()
{
}

void
TemperatureMarginsDialog::init_ui()
{
	builder_->get_widget( "hscale-average-temperature",
		hscale_temperature_average_value_);
	builder_->get_widget( "spinbutton-temperature-spread",
		spinbutton_temperature_spread_value_);
}

void
TemperatureMarginsDialog::connect_signals()
{
	hscale_temperature_average_value_->signal_format_value().connect(
		sigc::ptr_fun(&format_temperature_value));
}

void
TemperatureMarginsDialog::update_scanner_state(const Scanner::State& state)
{
	if (!ui_state_initiated_) {
		set_temperature_margins(state);
		ui_state_initiated_ = true;
	}

	Scanner::ManagerState mstate = state.manager_state();

	switch (mstate.run()) {
	case RUN_BACKGROUND:
		set_sensitive(true);
		break;
	case RUN_NONE:
	default:
		set_sensitive(false);
		break;
	}			
}

void
TemperatureMarginsDialog::on_response(int result)
{
	switch (result) {
	case 0:
		{
			Scanner::SharedManager manager = Scanner::Manager::instance();

			double temperature = hscale_temperature_average_value_->get_value();
			double spread = spinbutton_temperature_spread_value_->get_value();

			bool res = manager->set_temperature_margins( temperature, spread);
			if (!res) {
				InfoDialog dialog( *this, _("<b>Unable to change temperature margins!</b>"));
				dialog.set_secondary_text(_("Scanner to locked for some reason."));
				dialog.run();
			}
		}
		break;
	default:
		break;
	}
	hide();
}

void
TemperatureMarginsDialog::set_temperature_margins(const Scanner::State& state)
{
	double average, spread;
	state.temperature_margins( average, spread);
	hscale_temperature_average_value_->set_value(average);
	spinbutton_temperature_spread_value_->set_value(spread);
}

void
TemperatureMarginsDialog::block_interface(bool block)
{
	switch (block) {
	case true:
		break;
	case false:
		break;
	default:
		break;
	}
}

TemperatureMarginsDialog*
TemperatureMarginsDialog::create()
{
	TemperatureMarginsDialog* dialog = 0;

	std::string filename(builder_temperature_dialog_filename);

	Glib::RefPtr<Gtk::Builder> builder = create_from_file(filename);

	if (builder)
		builder->get_widget_derived( "dialog-window", dialog);

	return dialog;
}

} // namespace UI

} // namespace ScanAmati
