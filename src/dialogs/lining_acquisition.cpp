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

#include <gtkmm/button.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/expander.h>

// files from src directory begin
#include "scanner/manager.hpp"
#include "application.hpp"
#include "global_strings.hpp"
// files from src directory end

#include "chips_lining.hpp"
#include "utils.hpp"
#include "lining_acquisition.hpp"

namespace {

const struct ModelColumns : public Gtk::TreeModelColumnRecord {
	Gtk::TreeModelColumn<Glib::ustring> label;
	Gtk::TreeModelColumn<Glib::ustring> label_code;
	Gtk::TreeModelColumn<gchar> code;

	ModelColumns() { add(label); add(label_code); add(code); }
} model_columns;

}

namespace ScanAmati {

namespace UI {

LiningAcquisitionDialog::LiningAcquisitionDialog( BaseObjectType* cobject,
	const Glib::RefPtr<Gtk::Builder>& builder)
	:
	ScannerTemplateDialog( cobject, builder, "lining-acquisition"),
	spinbutton_adc_count_(0),
	button_chips_(0),
	button_start_(0),
	button_stop_(0),
	expander_accuracy_(0),
	radiobutton_accuracy_rough_(0),
	radiobutton_accuracy_optimal_(0),
	radiobutton_accuracy_precise_(0),
	accuracy_(Scanner::LINING_ACCURACY_OPTIMAL),
	lining_data_ready_(false),
	chip_codes_( Scanner::array_chip_codes, Scanner::array_chip_codes + SCANNER_CHIPS)
{
	init_ui();

	connect_signals();

	show_all_children();
}

LiningAcquisitionDialog::~LiningAcquisitionDialog()
{
}

void
LiningAcquisitionDialog::init_ui()
{
	builder_->get_widget( "spinbutton-count", spinbutton_adc_count_);
	builder_->get_widget( "button-chips", button_chips_);
	builder_->get_widget( "button-start", button_start_);
	builder_->get_widget( "button-stop", button_stop_);

	builder_->get_widget( "radiobutton-rough", radiobutton_accuracy_rough_);
	builder_->get_widget( "radiobutton-optimal", radiobutton_accuracy_optimal_);
	builder_->get_widget( "radiobutton-precise", radiobutton_accuracy_precise_);

	builder_->get_widget( "expander-accuracy", expander_accuracy_);
	if (app.extend) {
		button_chips_->set_sensitive(true);
		expander_accuracy_->set_sensitive(true);
		radiobutton_accuracy_rough_->set_active(true);
		accuracy_ = Scanner::LINING_ACCURACY_ROUGH;
	}
	
	Glib::RefPtr<Glib::Object> obj = builder_->get_object("adjustment-count");
	if (obj)
		adjustment_lining_counts_ = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(obj);

	if (adjustment_lining_counts_)
		spinbutton_adc_count_->set_value(adjustment_lining_counts_->get_value());
}

void
LiningAcquisitionDialog::connect_signals()
{
	button_chips_->signal_clicked().connect(
		sigc::mem_fun( *this, &LiningAcquisitionDialog::on_chips));
	button_start_->signal_clicked().connect(
		sigc::mem_fun( *this, &LiningAcquisitionDialog::on_start));
	button_stop_->signal_clicked().connect(
		sigc::mem_fun( *this, &LiningAcquisitionDialog::on_stop));

	radiobutton_accuracy_rough_->signal_toggled().connect(sigc::bind(
		sigc::mem_fun( *this, &LiningAcquisitionDialog::on_accuracy_changed),
		Scanner::LINING_ACCURACY_ROUGH));
	radiobutton_accuracy_optimal_->signal_toggled().connect(sigc::bind(
		sigc::mem_fun( *this, &LiningAcquisitionDialog::on_accuracy_changed),
		Scanner::LINING_ACCURACY_OPTIMAL));
	radiobutton_accuracy_precise_->signal_toggled().connect(sigc::bind(
		sigc::mem_fun( *this, &LiningAcquisitionDialog::on_accuracy_changed),
		Scanner::LINING_ACCURACY_PRECISE));
}

void
LiningAcquisitionDialog::block_interface(bool block)
{
	switch (block) {
	case true:
		button_start_->set_sensitive(false);
		button_stop_->set_sensitive(true);
		spinbutton_adc_count_->set_sensitive(false);
		if (app.extend) {
			button_chips_->set_sensitive(false);
			expander_accuracy_->set_sensitive(false);
		}
		break;
	case false:
		button_start_->set_sensitive(true);
		button_stop_->set_sensitive(false);
		spinbutton_adc_count_->set_sensitive(true);
		if (app.extend) {
			button_chips_->set_sensitive(true);
			expander_accuracy_->set_sensitive(true);
		}
		break;
	default:
		break;
	}
}

void
LiningAcquisitionDialog::on_chips()
{
	ChipsLiningDialog* dialog = ChipsLiningDialog::create(chip_codes_);
	if (dialog) {
		selection_chips_ = dialog->selection_chips();
		selection_chips_->signal_changed().connect(sigc::mem_fun(
			*this, &LiningAcquisitionDialog::on_selection_changed));
		dialog->run();
	}
	delete dialog;
}

void
LiningAcquisitionDialog::on_selection_changed()
{
	chip_codes_.clear();

	selection_chips_->selected_foreach_iter(
		sigc::mem_fun(*this, &LiningAcquisitionDialog::row_selected));

	// Start the acquisiton only if one or more chips have been selected
	button_start_->set_sensitive(!chip_codes_.empty());
}

void
LiningAcquisitionDialog::row_selected(const Gtk::TreeModel::iterator& it)
{
	Gtk::TreeModel::Row row = *it;
	chip_codes_.push_back(row[model_columns.code]);
}

void
LiningAcquisitionDialog::on_response(int result)
{
	hide();
}

void
LiningAcquisitionDialog::on_start()
{
	gint16 count = spinbutton_adc_count_->get_value_as_int();
	Scanner::SharedManager manager = Scanner::Manager::instance();

	Scanner::AcquisitionParameters params;
	params.lining_accuracy_type = accuracy_;
	params.lining_count = count;
	params.value = chip_codes_;
	manager->run( RUN_LINING_ACQUISITION, params);
	block_interface(true);
}

void
LiningAcquisitionDialog::on_stop()
{
	Scanner::SharedManager manager = Scanner::Manager::instance();
	manager->stop(false);
}

void
LiningAcquisitionDialog::on_accuracy_changed(
	Scanner::LiningAccuracyType accuracy)
{
	accuracy_ = accuracy;
}

void
LiningAcquisitionDialog::update_scanner_state(const Scanner::State& state)
{
	Scanner::ManagerState mstate = state.manager_state();

	switch (mstate.run()) {
	case RUN_LINING_ACQUISITION:
		block_interface(true);
		if (mstate.process_finished()) {
			OFLOG_DEBUG( app.log, "Lining acquisition finished");
			lining_data_ready_ = true;
		}
		break;
	case RUN_BACKGROUND:
		block_interface(false);
		if (lining_data_ready_) {
			signal_lining_ready_();
			lining_data_ready_ = false;
		}
		break;
	case RUN_NONE:
	default:
		block_interface(true);
		button_start_->set_sensitive(false);
	}
}

sigc::signal<void>
LiningAcquisitionDialog::signal_lining_ready()
{
	return signal_lining_ready_;
}

LiningAcquisitionDialog*
LiningAcquisitionDialog::create()
{
	LiningAcquisitionDialog* dialog = 0;
	std::string filename(builder_lining_acquisition_dialog_filename);

	Glib::RefPtr<Gtk::Builder> builder = create_from_file(filename);

	if (builder)
		//Get the GtkBuilder-instantiated dialog
		builder->get_widget_derived( "dialog-window", dialog);

	return dialog;
}

} // namespace UI

} // namespace ScanAmati
