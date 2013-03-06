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

#include "utils.hpp"
#include "lining_acquisition.hpp"

namespace ScanAmati {

namespace UI {

LiningAcquisitionDialog::LiningAcquisitionDialog( BaseObjectType* cobject,
	const Glib::RefPtr<Gtk::Builder>& builder)
	:
	ScannerTemplateDialog( cobject, builder, "lining-acquisition"),
	spinbutton_adc_count_(0),
	button_start_(0),
	button_stop_(0),
	expander_accuracy_(0),
	radiobutton_accuracy_rough_(0),
	radiobutton_accuracy_optimal_(0),
	radiobutton_accuracy_precise_(0),
	accuracy_(Scanner::LINING_ACCURACY_OPTIMAL)
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
	builder_->get_widget( "button-start", button_start_);
	builder_->get_widget( "button-stop", button_stop_);

	builder_->get_widget( "radiobutton-rough", radiobutton_accuracy_rough_);
	builder_->get_widget( "radiobutton-optimal", radiobutton_accuracy_optimal_);
	builder_->get_widget( "radiobutton-precise", radiobutton_accuracy_precise_);

	builder_->get_widget( "expander-accuracy", expander_accuracy_);
	if (app.extend)
		expander_accuracy_->set_sensitive(true);
}

void
LiningAcquisitionDialog::connect_signals()
{
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
	manager->run( RUN_LINING_ACQUISITION, params);
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
		button_start_->set_sensitive(false);
		button_stop_->set_sensitive(true);
		spinbutton_adc_count_->set_sensitive(false);
		if (mstate.process_finished()) {
			OFLOG_DEBUG( app.log, "Lining acquisition finished");
			signal_lining_ready_();
		}
		if (app.extend)
			expander_accuracy_->set_sensitive(false);
		break;
	case RUN_BACKGROUND:
		button_start_->set_sensitive(true);
		button_stop_->set_sensitive(false);
		spinbutton_adc_count_->set_sensitive(true);
		if (app.extend)
			expander_accuracy_->set_sensitive(true);
		break;
	case RUN_NONE:
	default:
		button_start_->set_sensitive(false);
		button_stop_->set_sensitive(false);
		spinbutton_adc_count_->set_sensitive(false);
		if (app.extend)
			expander_accuracy_->set_sensitive(false);
		break;
	}
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