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
#include <gtkmm/frame.h>
#include <gtkmm/alignment.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/radiobuttongroup.h>

// files from src directory begin
#include "scanner/builtin_chip_capacities.hpp"
#include "scanner/manager.hpp"

#include "widgets/utils.hpp"
#include "global_strings.hpp"
// files from src directory end

#include "utils.hpp"
#include "chip_capacities.hpp"

namespace ScanAmati {

namespace UI {

ChipCapacitiesDialog::ChipCapacitiesDialog( BaseObjectType* cobject,
	const Glib::RefPtr<Gtk::Builder>& builder)
	:
	ScannerTemplateDialog( cobject, builder, "chip-capacities"),
	capacity_(SCANNER_DEFAULT_CHIP_CAPACITY)
{
	init_ui();

	connect_signals();

	show_all_children();
}

ChipCapacitiesDialog::~ChipCapacitiesDialog()
{
}

void
ChipCapacitiesDialog::init_ui()
{
}

void
ChipCapacitiesDialog::connect_signals()
{
	std::vector<double> capacities = Scanner::BuiltinCapacities::capacities();

	for ( std::vector<double>::const_iterator iter = capacities.begin();
		iter != capacities.end(); ++iter) {

		Gtk::RadioButton* button;
		Glib::ustring name = Glib::ustring::compose( "radiobutton%1",
			Glib::ustring::format(iter - capacities.begin()));

		builder_->get_widget( name, button);

		insert_into_buttons_map( *iter, button);

		button->signal_toggled().connect( sigc::bind(sigc::mem_fun( *this,
			&ChipCapacitiesDialog::on_capacity_changed), *iter));
	}
}

void
ChipCapacitiesDialog::on_capacity_changed(double capacity)
{
	capacity_ = capacity;
}

void
ChipCapacitiesDialog::update_scanner_state(const Scanner::State& state)
{
	if (!ui_state_initiated_) {
		set_current_capacity(state);
		ui_state_initiated_ = true;
	}

	Scanner::ManagerState mstate = state.manager_state();

	switch (mstate.run()) {
	case RUN_BACKGROUND:
		set_sensitive(true);
		break;
	default:
		set_sensitive(false);
		break;
	}
}

void
ChipCapacitiesDialog::on_response(int res)
{
	switch (res) {
	case 0:
		{
			Scanner::Command* com = Scanner::Commands::create(capacity_);

			Scanner::SharedManager manager = Scanner::Manager::instance();
			Scanner::AcquisitionParameters params(com);
			params.value = capacity_;
			manager->run( RUN_COMMANDS, params);
		}
		break;
	default:
		break;
	}
	hide();
}

void
ChipCapacitiesDialog::set_current_capacity(const Scanner::State& state)
{
	Gtk::RadioButton* button = buttons_map_[state.capacity()];
	if (button)
		button->set_active(true);
}

void
ChipCapacitiesDialog::insert_into_buttons_map(
	double capacity, Gtk::RadioButton* button)
{
	std::pair< double, Gtk::RadioButton*> pair( capacity, button);
	buttons_map_.insert(pair);
}

ChipCapacitiesDialog*
ChipCapacitiesDialog::create()
{
	ChipCapacitiesDialog* dialog = 0;
	std::string filename(builder_capacities_dialog_filename);

	Glib::RefPtr<Gtk::Builder> builder = create_from_file(filename);

	if (builder)
		//Get the GtkBuilder-instantiated dialog
		builder->get_widget_derived( "dialog-window", dialog);

	return dialog;
}

} // namespace UI

} // namespace ScanAmati
