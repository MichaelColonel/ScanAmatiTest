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
	button_acquisition_parameters_(0)
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

	builder_->get_widget( "button-acquisition-parameters", button_acquisition_parameters_);

	set_acquisition_info(acquisition_);

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
ImageAcquisitionDialog::on_acquisition_run()
{
	Scanner::SharedManager manager = Scanner::Manager::instance();

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
				Scanner::AcquisitionParameters params;
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
		case PROCESS_FINISH:
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
		dialog->set_parameters(acquisition_);
		dialog->run();
		dialog->get_parameters(acquisition_);
		set_acquisition_info(acquisition_);
		delete dialog;
	}
}

void
ImageAcquisitionDialog::set_acquisition_info(
	const Scanner::AcquisitionParameters& params)
{
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

} // namespace UI

} // namespace ScanAmati
