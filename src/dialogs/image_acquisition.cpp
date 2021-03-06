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
#include <gtkmm/image.h>
#include <gtkmm/scale.h>
#include <gtkmm/button.h>
#include <gtkmm/combobox.h>
#include <gtkmm/entry.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/treemodelcolumn.h>

// files from src directory begin
#include "scanner/manager.hpp"
#include "scanner/x-ray.hpp"

#include "widgets/utils.hpp"

#include "preferences.hpp"
#include "application.hpp"
#include "icon_names.hpp"
#include "global_strings.hpp"
// files from src directory end

#include "acquisition_parameters.hpp"
#include "utils.hpp"

#include "image_acquisition.hpp"

namespace {

Gdk::Color color_red("red");
Gdk::Color color_yellow("yellow");

const char* conf_key_xray_high_voltage = "-dialog-xray-high-voltage";
const char* conf_key_xray_exposure_pos = "-dialog-xray-exposure-pos";
const char* conf_key_memory_size = "-dialog-memory-size";

} // namespace

namespace ScanAmati {

namespace UI {

class XrayCheckDialog : public Gtk::MessageDialog {
public:
	XrayCheckDialog(Gtk::Window& parent);
};

XrayCheckDialog::XrayCheckDialog(Gtk::Window& parent)
	:
	Gtk::MessageDialog( parent,
		_("If the x-ray source is ready press Yes otherwise press No"),
		false,
		Gtk::MESSAGE_QUESTION,
		Gtk::BUTTONS_YES_NO)
{
	set_title(_("X-ray Preparation"));

	Scanner::SharedManager manager = Scanner::Manager::instance();

	Scanner::Command* com =
		Scanner::Commands::create(Scanner::COMMAND_XRAY_CHECK_ON);

	Scanner::AcquisitionParameters params(com);
	manager->run( RUN_COMMANDS, params);
}

ImageAcquisitionDialog::ImageAcquisitionDialog( BaseObjectType* cobject,
	const Glib::RefPtr<Gtk::Builder>& builder)
	:
	ScannerTemplateDialog( cobject, builder, "image-acquisition"),
	scale_xray_voltage_(0),
	scale_xray_exposure_(0),
	button_acquisition_run_(0),
	button_acquisition_stop_(0),
	image_radiation_indicator_(0),
	checkbutton_array_movement_(0),
	checkbutton_xray_movement_(0),
	checkbutton_exposure_(0),
	checkbutton_acquisition_(0),
	radiobutton_memory_8mbytes_(0),
	radiobutton_memory_16mbytes_(0),
	radiobutton_memory_24mbytes_(0),
	radiobutton_memory_32mbytes_(0),
	button_acquisition_parameters_(0),
	data_ready_(false)
{
	init_ui();

	connect_signals();

	show_all_children();
}

ImageAcquisitionDialog::~ImageAcquisitionDialog()
{
}

void
ImageAcquisitionDialog::init_ui()
{
	exposures_ = XrayParameters::exposures();

	Glib::RefPtr<Glib::Object> obj;
	obj = builder_->get_object("adjustment-exposure");
	adjustment_xray_exposure_ = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(obj);
	adjustment_xray_exposure_->set_upper(exposures_.size() - 1);

	obj = builder_->get_object("adjustment-high-voltage");
	adjustment_xray_voltage_ = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(obj);

	builder_->get_widget( "hscale-high-voltage", scale_xray_voltage_);
	builder_->get_widget( "hscale-exposure", scale_xray_exposure_);

	Glib::RefPtr<Gdk::Colormap> colormap = get_default_colormap();

	colormap->alloc_color(color_red);
	colormap->alloc_color(color_yellow);

	builder_->get_widget( "button-run", button_acquisition_run_);
	button_acquisition_run_->modify_bg( Gtk::STATE_NORMAL, color_yellow);
	button_acquisition_run_->modify_bg( Gtk::STATE_PRELIGHT, color_yellow);

	builder_->get_widget( "button-stop", button_acquisition_stop_);
	button_acquisition_stop_->modify_bg( Gtk::STATE_NORMAL, color_red);
	button_acquisition_stop_->modify_bg( Gtk::STATE_PRELIGHT, color_red);

	builder_->get_widget( "image-radiation-indicator", image_radiation_indicator_);
	image_radiation_indicator_->set( Gtk::StockID(icon_radiation_inactive),
		Gtk::IconSize::from_name(icon_size_64x64));

	builder_->get_widget( "checkbutton-array-movement", checkbutton_array_movement_);
	builder_->get_widget( "checkbutton-xray-movement", checkbutton_xray_movement_);
	builder_->get_widget( "checkbutton-exposure", checkbutton_exposure_);
	builder_->get_widget( "checkbutton-acquisition", checkbutton_acquisition_);

	builder_->get_widget( "radiobutton-8MB", radiobutton_memory_8mbytes_);
	builder_->get_widget( "radiobutton-16MB", radiobutton_memory_16mbytes_);
	builder_->get_widget( "radiobutton-24MB", radiobutton_memory_24mbytes_);
	builder_->get_widget( "radiobutton-32MB", radiobutton_memory_32mbytes_);
	
	builder_->get_widget( "button-acquisition-parameters", button_acquisition_parameters_);

	set_parameters(acquisition_);

	if (app.extend)
		button_acquisition_parameters_->set_sensitive(true);
}

void
ImageAcquisitionDialog::connect_signals()
{
	scale_xray_voltage_->signal_format_value().connect(
		sigc::mem_fun( *this, &ImageAcquisitionDialog::on_format_voltage_value));
	scale_xray_exposure_->signal_format_value().connect(
		sigc::mem_fun( *this, &ImageAcquisitionDialog::on_format_exposure_value));

	adjustment_xray_voltage_->signal_value_changed().connect(
		sigc::mem_fun( *this, &ImageAcquisitionDialog::on_xray_parameters_changed));
	adjustment_xray_exposure_->signal_value_changed().connect(
		sigc::mem_fun( *this, &ImageAcquisitionDialog::on_xray_parameters_changed));

	button_acquisition_run_->signal_clicked().connect(
		sigc::mem_fun( *this, &ImageAcquisitionDialog::on_acquisition_run));
	button_acquisition_stop_->signal_clicked().connect(
		sigc::mem_fun( *this, &ImageAcquisitionDialog::on_acquisition_stop));

	button_acquisition_parameters_->signal_clicked().connect(
		sigc::mem_fun( *this, &ImageAcquisitionDialog::on_acquisition_parameters));

	radiobutton_memory_8mbytes_->signal_toggled().connect(sigc::bind(
		sigc::mem_fun( *this, &ImageAcquisitionDialog::on_memory_size_changed),
		radiobutton_memory_8mbytes_, SCANNER_MEMORY_BANK));
	radiobutton_memory_16mbytes_->signal_toggled().connect(sigc::bind(
		sigc::mem_fun( *this, &ImageAcquisitionDialog::on_memory_size_changed),
		radiobutton_memory_16mbytes_, 2 * SCANNER_MEMORY_BANK));
	radiobutton_memory_24mbytes_->signal_toggled().connect(sigc::bind(
		sigc::mem_fun( *this, &ImageAcquisitionDialog::on_memory_size_changed),
		radiobutton_memory_24mbytes_, 3 * SCANNER_MEMORY_BANK));
	radiobutton_memory_32mbytes_->signal_toggled().connect(sigc::bind(
		sigc::mem_fun( *this, &ImageAcquisitionDialog::on_memory_size_changed),
		radiobutton_memory_32mbytes_, 4 * SCANNER_MEMORY_BANK));

	radiobutton_memory_32mbytes_->set_active();
}

void
ImageAcquisitionDialog::on_response(int result)
{
	switch (result) {
	case 0:
		hide();
		break;
	default:
		break;
	}
	hide();
}

void
ImageAcquisitionDialog::block_interface(bool block)
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

void
ImageAcquisitionDialog::on_acquisition_run()
{
	get_parameters(acquisition_);

	Scanner::SharedManager manager = Scanner::Manager::instance();

	signal_with_acquisition_(acquisition_.with_acquisition);

	if (acquisition_.with_exposure) {
		XrayCheckDialog dialog(*this);
		int res = dialog.run();

		switch (res) {
		case Gtk::RESPONSE_YES:
			manager->run( RUN_IMAGE_ACQUISITION, acquisition_);
			button_acquisition_run_->set_sensitive(false);
			button_acquisition_stop_->set_sensitive(false);
			break;
		case Gtk::RESPONSE_NO:
			{
				Scanner::Command* com =
					Scanner::Commands::create(Scanner::COMMAND_XRAY_CHECK_OFF);
				Scanner::AcquisitionParameters params(com);
				manager->run( RUN_COMMANDS, params);
			}
			break;
		default:
			break;
		}
	}
	else {
		manager->run( RUN_IMAGE_ACQUISITION, acquisition_);
		button_acquisition_run_->set_sensitive(false);
		button_acquisition_stop_->set_sensitive(false);
	}
}

void
ImageAcquisitionDialog::on_acquisition_stop()
{
}

void
ImageAcquisitionDialog::on_xray_parameters_changed()
{
	size_t expo_pos = static_cast<size_t>(scale_xray_exposure_->get_value());
	int voltage = static_cast<int>(scale_xray_voltage_->get_value());
	XrayExposureRange range = XrayParameters::exposure_range(voltage);

	size_t pos = std::find( exposures_.begin(), exposures_.end(),
		range.second) - exposures_.begin();

	double exposure;
	if (expo_pos > pos) {
		adjustment_xray_exposure_->set_value(double(pos));
		exposure = exposures_[pos];
	}
	else {
		exposure = exposures_[expo_pos];
	}

	adjustment_xray_exposure_->set_upper(double(pos));

	XrayParameters params( voltage, exposure);
}

void
ImageAcquisitionDialog::update_scanner_state(const Scanner::State& state)
{
	switch (state.manager_state().run()) {
	case RUN_IMAGE_ACQUISITION:
		switch (state.manager_state().process()) {
		case PROCESS_START:
			button_acquisition_run_->set_sensitive(false);
			button_acquisition_stop_->set_sensitive(true);
	
			scale_xray_exposure_->set_sensitive(false);
			scale_xray_voltage_->set_sensitive(false);

			if (app.extend)
				button_acquisition_parameters_->set_sensitive(false);

			break;
		case PROCESS_EXPOSURE:
			button_acquisition_run_->set_sensitive(false);
			button_acquisition_stop_->set_sensitive(true);

			image_radiation_indicator_->set(
				Gtk::StockID(icon_radiation_active),
				Gtk::IconSize::from_name(icon_size_64x64));
			break;
		case PROCESS_ACQUISITION:
			image_radiation_indicator_->set(
				Gtk::StockID(icon_radiation_inactive),
				Gtk::IconSize::from_name(icon_size_64x64));
			break;
		case PROCESS_PARKING:
			break;
		case PROCESS_FINISH:
			data_ready_ = true;
			std::cout << "data_ready" << std::endl;
			button_acquisition_run_->set_sensitive(false);
			button_acquisition_stop_->set_sensitive(false);
			image_radiation_indicator_->set(
				Gtk::StockID(icon_radiation_inactive),
				Gtk::IconSize::from_name(icon_size_64x64));
			break;
		case PROCESS_ABORTED:
		default:
			break;
		}
		break;
	case RUN_BACKGROUND:
			if (data_ready_) {
				std::cout << "data_ready_" << std::endl;
				signal_scanner_data_ready_();
				data_ready_ = false;
			}
			button_acquisition_run_->set_sensitive(true);
			button_acquisition_stop_->set_sensitive(false);
	
			scale_xray_exposure_->set_sensitive(true);
			scale_xray_voltage_->set_sensitive(true);

			if (app.extend)
				button_acquisition_parameters_->set_sensitive(true);
		break;
	case RUN_NONE:
	default:
			button_acquisition_run_->set_sensitive(false);
			button_acquisition_stop_->set_sensitive(false);
	
			scale_xray_exposure_->set_sensitive(false);
			scale_xray_voltage_->set_sensitive(false);

			if (app.extend)
				button_acquisition_parameters_->set_sensitive(false);
		break;
	}
}

void
ImageAcquisitionDialog::on_memory_size_changed(
	const Gtk::RadioButton* button, size_t memory_size)
{
	if (button->get_active()) {
		OFLOG_DEBUG( app.log, "Memory size in bytes: " << memory_size);
		acquisition_.acquisition.memory_size = memory_size;
	}
}

Glib::ustring
ImageAcquisitionDialog::on_format_voltage_value(double voltage)
{
	int hv = static_cast<int>(voltage);
	return format_value<int>( hv, _("kV"));
}

Glib::ustring
ImageAcquisitionDialog::on_format_exposure_value(double exposure_pos)
{
	size_t pos = static_cast<size_t>(exposure_pos);
	return format_value<double>( exposures_[pos], _("mAs"));
}

void
ImageAcquisitionDialog::on_acquisition_parameters()
{
	AcquisitionParametersDialog* dialog = AcquisitionParametersDialog::create();
	if (dialog) {
		get_parameters(acquisition_);
		dialog->set_parameters(acquisition_);
		dialog->run();
		dialog->get_parameters(acquisition_);
		set_parameters(acquisition_);
		delete dialog;
	}
}

void
ImageAcquisitionDialog::get_parameters(
	Scanner::AcquisitionParameters& params)
{
	bool array = checkbutton_array_movement_->get_active();
	bool xray = checkbutton_xray_movement_->get_active();

	if (array && xray)
		params.movement_type = Scanner::MOVEMENT_BOTH;
	else if (!array && xray)
		params.movement_type = Scanner::MOVEMENT_XRAY;
	else if (array && !xray)
		params.movement_type = Scanner::MOVEMENT_ARRAY;
	else if (!array && !xray)
		params.movement_type = Scanner::MOVEMENT_NONE;
	else
		params.movement_type = Scanner::MOVEMENT_NONE;

	params.with_exposure = checkbutton_exposure_->get_active();
	params.with_acquisition = checkbutton_acquisition_->get_active();
}

void
ImageAcquisitionDialog::set_parameters(
	const Scanner::AcquisitionParameters& params)
{
	acquisition_ = params;

	bool array, xray;
	switch (params.movement_type) {
	case Scanner::MOVEMENT_BOTH:
		array = xray = true;
		break;
	case Scanner::MOVEMENT_XRAY:
		xray = true;
		array = false;
		break;
	case Scanner::MOVEMENT_ARRAY:
		array = true;
		xray = false;
		break;
	case Scanner::MOVEMENT_NONE:
	default:
		array = xray = false;
		break;
	}

	checkbutton_array_movement_->set_active(array);
	checkbutton_xray_movement_->set_active(xray);

	checkbutton_exposure_->set_active(params.with_exposure);
	checkbutton_acquisition_->set_active(params.with_acquisition);

	switch (params.acquisition.memory_size) {
	case SCANNER_MEMORY_BANK:
		radiobutton_memory_8mbytes_->set_active();
		break;
	case 2 * SCANNER_MEMORY_BANK:
		radiobutton_memory_16mbytes_->set_active();
		break;
	case 3 * SCANNER_MEMORY_BANK:
		radiobutton_memory_24mbytes_->set_active();
		break;
	case 4 * SCANNER_MEMORY_BANK:
		radiobutton_memory_32mbytes_->set_active();
		break;
	default:
		break;
	}
}

ImageAcquisitionDialog*
ImageAcquisitionDialog::create()
{
	ImageAcquisitionDialog* dialog = 0;
	std::string filename(builder_image_acquisition_dialog_filename);

	Glib::RefPtr<Gtk::Builder> builder = create_from_file(filename);

	if (builder)
		//Get the GtkBuilder-instantiated dialog
		builder->get_widget_derived( "dialog-window", dialog);

	return dialog;
}

sigc::signal< void, bool>
ImageAcquisitionDialog::signal_with_acquisition()
{
	return signal_with_acquisition_;
}

sigc::signal<void>
ImageAcquisitionDialog::signal_scanner_data_ready()
{
	return signal_scanner_data_ready_;
}

} // namespace UI

} // namespace ScanAmati
