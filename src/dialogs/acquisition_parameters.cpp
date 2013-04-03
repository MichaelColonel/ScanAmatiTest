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

#include <gtkmm/frame.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/button.h>
#include <gtkmm/combobox.h>
#include <gtkmm/entry.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treemodelcolumn.h>

#include <iostream>

/* files from src directory begin */
#include "scanner/defines.hpp"
#include "widgets/utils.hpp"
#include "global_strings.hpp"
#include "application.hpp"
/* files from src directory end */

#include "utils.hpp"
#include "acquisition_parameters.hpp"

namespace ScanAmati {

namespace UI {

AcquisitionParametersDialog::AcquisitionParametersDialog(
	BaseObjectType* cobject,
	const Glib::RefPtr<Gtk::Builder>& builder)
	:
	Gtk::Dialog(cobject),
	builder_(builder),
	checkbutton_array_movement_(0),
	checkbutton_xray_movement_(0),
	checkbutton_exposure_(0),
	checkbutton_acquisition_(0),
	spinbutton_forward_scanning_time_(0),
	spinbutton_forward_steps_(0),
	spinbutton_forward_speed_array_freq_(0),
	spinbutton_forward_speed_xray_freq_(0),
	combobox_forward_speed_array_mode_(0),
	combobox_forward_speed_xray_mode_(0),
	spinbutton_forward_delay_array_(0),
	spinbutton_forward_delay_xray_(0),
	spinbutton_reverse_scanning_time_(0),
	spinbutton_reverse_steps_(0),
	spinbutton_reverse_speed_array_freq_(0),
	spinbutton_reverse_speed_xray_freq_(0),
	combobox_reverse_speed_array_mode_(0),
	combobox_reverse_speed_xray_mode_(0),
	spinbutton_reverse_delay_array_(0),
	spinbutton_reverse_delay_xray_(0),
	checkbutton_equal_movement_(0),
	frame_reverse_(0),
	radiobutton_width_full_(0),
	radiobutton_width_half_(0),
	radiobutton_width_quarter_(0),
	radiobutton_calibration_rough_(0),
	radiobutton_calibration_good_(0),
	radiobutton_calibration_better_(0),
	radiobutton_calibration_best_(0),
	radiobutton_filter_lanczos_(0),
	radiobutton_filter_cubic_(0),
	radiobutton_filter_bessel_(0),
	radiobutton_filter_quadratic_(0),
	radiobutton_intensity_original_(0),
	radiobutton_intensity_linear_(0),
	radiobutton_intensity_logarithmic_(0),
	radiobutton_memory_builtin_(0),
	radiobutton_memory_accurate_(0),
	radiobutton_memory_custom_(0),
	spinbutton_image_height_(0),
	combobox_builtin_memory_(0),
	spinbutton_accurate_memory_(0),
	entry_custom_memory_(0),
	width_type_(Scanner::WIDTH_FULL),
	calibration_type_(Scanner::CALIBRATION_ROUGH),
	intensity_type_(Scanner::INTENSITY_ORIGINAL),
	filter_type_(Magick::LanczosFilter)
{
	init_ui();

	connect_signals();

	show_all_children();
}

AcquisitionParametersDialog::~AcquisitionParametersDialog()
{
}

void
AcquisitionParametersDialog::init_ui()
{
	builder_->get_widget( "checkbutton-array-movement", checkbutton_array_movement_);
	builder_->get_widget( "checkbutton-xray-movement", checkbutton_xray_movement_);
	builder_->get_widget( "checkbutton-exposure", checkbutton_exposure_);
	builder_->get_widget( "checkbutton-acquisition", checkbutton_acquisition_);

	builder_->get_widget( "spinbutton-forward-scanning-time",
		spinbutton_forward_scanning_time_);
	builder_->get_widget( "spinbutton-forward-steps",
		spinbutton_forward_steps_);
	builder_->get_widget( "spinbutton-forward-speed-array-freq",
		spinbutton_forward_speed_array_freq_);
	builder_->get_widget( "spinbutton-forward-speed-xray-freq",
		spinbutton_forward_speed_xray_freq_);

	builder_->get_widget( "combobox-forward-speed-array-mode",
		combobox_forward_speed_array_mode_);

	Glib::RefPtr<Gtk::ListStore> liststore = create_step_mode_liststore();
	combobox_forward_speed_array_mode_->property_model() = liststore;
	combobox_forward_speed_array_mode_->set_active(0);

	builder_->get_widget( "combobox-forward-speed-xray-mode",
		combobox_forward_speed_xray_mode_);
	liststore = create_step_mode_liststore();
	combobox_forward_speed_xray_mode_->property_model() = liststore;
	combobox_forward_speed_xray_mode_->set_active(0);

	builder_->get_widget( "spinbutton-forward-delay-array",
		spinbutton_forward_delay_array_);

	builder_->get_widget( "spinbutton-forward-delay-xray",
		spinbutton_forward_delay_xray_);

	builder_->get_widget( "spinbutton-reverse-scanning-time",
		spinbutton_reverse_scanning_time_);
	builder_->get_widget( "spinbutton-reverse-steps",
		spinbutton_reverse_steps_);
	builder_->get_widget( "spinbutton-reverse-speed-array-freq",
		spinbutton_reverse_speed_array_freq_);
	builder_->get_widget( "spinbutton-reverse-speed-xray-freq",
		spinbutton_reverse_speed_xray_freq_);

	builder_->get_widget( "combobox-reverse-speed-array-mode",
		combobox_reverse_speed_array_mode_);
	liststore = create_step_mode_liststore();
	combobox_reverse_speed_array_mode_->property_model() = liststore;
	combobox_reverse_speed_array_mode_->set_active(0);

	builder_->get_widget( "combobox-reverse-speed-xray-mode",
		combobox_reverse_speed_xray_mode_);
	liststore = create_step_mode_liststore();
	combobox_reverse_speed_xray_mode_->property_model() = liststore;
	combobox_reverse_speed_xray_mode_->set_active(0);

	builder_->get_widget( "spinbutton-reverse-delay-array",
		spinbutton_reverse_delay_array_);

	builder_->get_widget( "spinbutton-reverse-delay-xray",
		spinbutton_reverse_delay_xray_);

	builder_->get_widget( "checkbutton-equal-movement",
		checkbutton_equal_movement_);
	builder_->get_widget( "frame-reverse", frame_reverse_);

	builder_->get_widget( "radiobutton-width-full", radiobutton_width_full_);
	builder_->get_widget( "radiobutton-width-half", radiobutton_width_half_);
	builder_->get_widget( "radiobutton-width-quarter", radiobutton_width_quarter_);

	builder_->get_widget( "radiobutton-calibration-rough", radiobutton_calibration_rough_);
	builder_->get_widget( "radiobutton-calibration-good", radiobutton_calibration_good_);
	builder_->get_widget( "radiobutton-calibration-better", radiobutton_calibration_better_);
	builder_->get_widget( "radiobutton-calibration-best", radiobutton_calibration_best_);

	builder_->get_widget( "radiobutton-filter-lanczos", radiobutton_filter_lanczos_);
	builder_->get_widget( "radiobutton-filter-cubic", radiobutton_filter_cubic_);
	builder_->get_widget( "radiobutton-filter-bessel", radiobutton_filter_bessel_);
	builder_->get_widget( "radiobutton-filter-quadratic", radiobutton_filter_quadratic_);

	builder_->get_widget( "radiobutton-intensity-original", radiobutton_intensity_original_);
	builder_->get_widget( "radiobutton-intensity-linear", radiobutton_intensity_linear_);
	builder_->get_widget( "radiobutton-intensity-logarithmic", radiobutton_intensity_logarithmic_);

	builder_->get_widget( "radiobutton-builtin-memory", radiobutton_memory_builtin_);
	builder_->get_widget( "radiobutton-accurate-memory", radiobutton_memory_accurate_);
	builder_->get_widget( "radiobutton-custom-memory", radiobutton_memory_custom_);

	builder_->get_widget( "spinbutton-image-height", spinbutton_image_height_);
	builder_->get_widget( "combobox-builtin-memory", combobox_builtin_memory_);

	create_memory_size_liststore();
	combobox_builtin_memory_->property_model() = liststore_memory_size_;
	combobox_builtin_memory_->set_active(0);

	builder_->get_widget( "spinbutton-accurate-memory", spinbutton_accurate_memory_);
	builder_->get_widget( "entry-custom-memory", entry_custom_memory_);
}

void
AcquisitionParametersDialog::connect_signals()
{
	radiobutton_width_full_->signal_toggled().connect(sigc::bind(
		sigc::mem_fun( *this, &AcquisitionParametersDialog::on_image_width_changed),
		radiobutton_width_full_,
		Scanner::WIDTH_FULL));
	radiobutton_width_half_->signal_toggled().connect(sigc::bind(
		sigc::mem_fun( *this, &AcquisitionParametersDialog::on_image_width_changed),
		radiobutton_width_half_,
		Scanner::WIDTH_HALF));
	radiobutton_width_quarter_->signal_toggled().connect(sigc::bind(
		sigc::mem_fun( *this, &AcquisitionParametersDialog::on_image_width_changed),
		radiobutton_width_quarter_,
		Scanner::WIDTH_QUARTER));
	radiobutton_width_full_->toggled();

	radiobutton_calibration_rough_->signal_toggled().connect(sigc::bind(
		sigc::mem_fun( *this, &AcquisitionParametersDialog::on_image_calibration_changed),
		radiobutton_calibration_rough_,
		Scanner::CALIBRATION_ROUGH));
	radiobutton_calibration_good_->signal_toggled().connect(sigc::bind(
		sigc::mem_fun( *this, &AcquisitionParametersDialog::on_image_calibration_changed),
		radiobutton_calibration_good_,
		Scanner::CALIBRATION_GOOD));
	radiobutton_calibration_better_->signal_toggled().connect(sigc::bind(
		sigc::mem_fun( *this, &AcquisitionParametersDialog::on_image_calibration_changed),
		radiobutton_calibration_better_,
		Scanner::CALIBRATION_BETTER));
	radiobutton_calibration_best_->signal_toggled().connect(sigc::bind(
		sigc::mem_fun( *this, &AcquisitionParametersDialog::on_image_calibration_changed),
		radiobutton_calibration_best_,
		Scanner::CALIBRATION_BEST));
	radiobutton_calibration_rough_->toggled();

	radiobutton_filter_lanczos_->signal_toggled().connect(sigc::bind(
		sigc::mem_fun( *this, &AcquisitionParametersDialog::on_image_filter_changed),
		radiobutton_filter_lanczos_,
		Magick::LanczosFilter));
	radiobutton_filter_cubic_->signal_toggled().connect(sigc::bind(
		sigc::mem_fun( *this, &AcquisitionParametersDialog::on_image_filter_changed),
		radiobutton_filter_cubic_,
		Magick::CubicFilter));
	radiobutton_filter_bessel_->signal_toggled().connect(sigc::bind(
		sigc::mem_fun( *this, &AcquisitionParametersDialog::on_image_filter_changed),
		radiobutton_filter_bessel_,
		Magick::BesselFilter));
	radiobutton_filter_quadratic_->signal_toggled().connect(sigc::bind(
		sigc::mem_fun( *this, &AcquisitionParametersDialog::on_image_filter_changed),
		radiobutton_filter_quadratic_,
		Magick::QuadraticFilter));
	radiobutton_filter_lanczos_->toggled();

	radiobutton_intensity_linear_->signal_toggled().connect(sigc::bind(
		sigc::mem_fun( *this, &AcquisitionParametersDialog::on_image_intensity_changed),
		radiobutton_intensity_linear_,
		Scanner::INTENSITY_LINEAR));
	radiobutton_intensity_original_->signal_toggled().connect(sigc::bind(
		sigc::mem_fun( *this, &AcquisitionParametersDialog::on_image_intensity_changed),
		radiobutton_intensity_original_,
		Scanner::INTENSITY_ORIGINAL));
	radiobutton_intensity_logarithmic_->signal_toggled().connect(sigc::bind(
		sigc::mem_fun( *this, &AcquisitionParametersDialog::on_image_intensity_changed),
		radiobutton_intensity_logarithmic_,
		Scanner::INTENSITY_LOGARITHMIC));
	radiobutton_intensity_original_->toggled();

	checkbutton_equal_movement_->signal_toggled().connect(sigc::mem_fun(
		*this, &AcquisitionParametersDialog::on_equal_movement_toggled));

	radiobutton_memory_builtin_->signal_toggled().connect(sigc::bind(
		sigc::mem_fun( *this, &AcquisitionParametersDialog::on_memory_type_changed),
		radiobutton_memory_builtin_,
		BUILT_IN));
	radiobutton_memory_accurate_->signal_toggled().connect(sigc::bind(
		sigc::mem_fun( *this, &AcquisitionParametersDialog::on_memory_type_changed),
		radiobutton_memory_accurate_,
		ACCURATE));
	radiobutton_memory_custom_->signal_toggled().connect(sigc::bind(
		sigc::mem_fun( *this, &AcquisitionParametersDialog::on_memory_type_changed),
		radiobutton_memory_custom_,
		CUSTOM));
	radiobutton_memory_builtin_->toggled();

	combobox_builtin_memory_->signal_changed().connect(
		sigc::mem_fun( *this, &AcquisitionParametersDialog::on_builtin_memory_changed));
	spinbutton_accurate_memory_->signal_value_changed().connect(
		sigc::mem_fun( *this, &AcquisitionParametersDialog::on_accurate_memory_changed));
}

void
AcquisitionParametersDialog::on_response(int result)
{
	switch (result) {
	case 0:
		break;
	case 1:
		break;
	default:
		break;
	}
	hide();
}

void
AcquisitionParametersDialog::on_equal_movement_toggled()
{
	bool state = checkbutton_equal_movement_->get_active();
	frame_reverse_->set_sensitive(!state);
}

void
AcquisitionParametersDialog::set_parameters(
	const Scanner::AcquisitionParameters& params)
{
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

	set_movements( params.acquisition.movement_forward,
		params.acquisition.movement_reverse);

	spinbutton_image_height_->set_value(params.acquisition.image_height);
	set_memory_size(params.acquisition.memory_size);

	switch (params.width_type) {
	case Scanner::WIDTH_FULL:
		radiobutton_width_full_->toggled();
		break;
	case Scanner::WIDTH_HALF:
		radiobutton_width_half_->toggled();
		break;
	case Scanner::WIDTH_QUARTER:
		radiobutton_width_quarter_->toggled();
		break;
	default:
		break;
	}

	switch (params.filter_type) {
	case Magick::LanczosFilter:
		radiobutton_filter_lanczos_->toggled();
		break;
	case Magick::CubicFilter:
		radiobutton_filter_cubic_->toggled();
		break;
	case Magick::QuadraticFilter:
		radiobutton_filter_quadratic_->toggled();
		break;
	case Magick::BesselFilter:
		radiobutton_filter_bessel_->toggled();
		break;
	default:
		break;
	}

	switch (params.calibration_type) {
	case Scanner::CALIBRATION_BEST:
		radiobutton_calibration_best_->toggled();
		break;
	case Scanner::CALIBRATION_BETTER:
		radiobutton_calibration_better_->toggled();
		break;
	case Scanner::CALIBRATION_GOOD:
		radiobutton_calibration_good_->toggled();
		break;
	case Scanner::CALIBRATION_ROUGH:
		radiobutton_calibration_rough_->toggled();
		break;
	default:
		break;
	}

	switch (params.intensity_type) {
	case Scanner::INTENSITY_LINEAR:
		radiobutton_intensity_linear_->toggled();
		break;
	case Scanner::INTENSITY_LOGARITHMIC:
		radiobutton_intensity_logarithmic_->toggled();
		break;
	case Scanner::INTENSITY_ORIGINAL:
		radiobutton_intensity_original_->toggled();
		break;
	default:
		break;
	}
}

void
AcquisitionParametersDialog::get_parameters(
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

	get_movements( params.acquisition.movement_forward,
		params.acquisition.movement_reverse);

	params.acquisition.image_height = spinbutton_image_height_->get_value_as_int();
	params.acquisition.memory_size = get_memory_size();
	params.intensity_type = intensity_type_;
	params.width_type = width_type_;
	params.calibration_type = calibration_type_;
	params.filter_type = filter_type_;
}

void
AcquisitionParametersDialog::set_movements( const Scanner::Movement& forward,
	const Scanner::Movement& reverse)
{
	// forward
	spinbutton_forward_scanning_time_->set_value(forward.time);
	spinbutton_forward_steps_->set_value(forward.steps);

	spinbutton_forward_speed_array_freq_->set_value(forward.array_speed.freq);
	set_step_mode( combobox_forward_speed_array_mode_, forward.array_speed.mode);
	spinbutton_forward_speed_xray_freq_->set_value(forward.xray_speed.freq);
	set_step_mode( combobox_forward_speed_xray_mode_, forward.xray_speed.mode);

	spinbutton_forward_delay_xray_->set_value(forward.xray_delay);
	spinbutton_forward_delay_array_->set_value(forward.xray_array_delay);

	// reverse
	spinbutton_reverse_scanning_time_->set_value(reverse.time);
	spinbutton_reverse_steps_->set_value(reverse.steps);

	spinbutton_reverse_speed_array_freq_->set_value(reverse.array_speed.freq);
	set_step_mode( combobox_reverse_speed_array_mode_, reverse.array_speed.mode);
	spinbutton_reverse_speed_xray_freq_->set_value(reverse.xray_speed.freq);
	set_step_mode( combobox_reverse_speed_xray_mode_, reverse.xray_speed.mode);

	spinbutton_reverse_delay_xray_->set_value(reverse.xray_delay);
	spinbutton_reverse_delay_array_->set_value(reverse.xray_array_delay);
}

void
AcquisitionParametersDialog::get_movements( Scanner::Movement& forward,
	Scanner::Movement& reverse)
{
	// forward
	forward.time = spinbutton_forward_scanning_time_->get_value();
	forward.steps = spinbutton_forward_steps_->get_value_as_int();

	forward.array_speed.freq = spinbutton_forward_speed_array_freq_->get_value_as_int();
	forward.array_speed.mode = get_step_mode(combobox_forward_speed_array_mode_);
	forward.xray_speed.freq = spinbutton_forward_speed_xray_freq_->get_value_as_int();
	forward.xray_speed.mode = get_step_mode(combobox_forward_speed_xray_mode_);

	forward.xray_delay = spinbutton_forward_delay_xray_->get_value_as_int();
	forward.xray_array_delay = spinbutton_forward_delay_array_->get_value_as_int();

	// reverse
	reverse.time = spinbutton_reverse_scanning_time_->get_value();
	reverse.steps = spinbutton_reverse_steps_->get_value_as_int();

	reverse.array_speed.freq = spinbutton_reverse_speed_array_freq_->get_value_as_int();
	reverse.array_speed.mode = get_step_mode(combobox_reverse_speed_array_mode_);
	reverse.xray_speed.freq = spinbutton_reverse_speed_xray_freq_->get_value_as_int();
	reverse.xray_speed.mode = get_step_mode(combobox_reverse_speed_xray_mode_);

	reverse.xray_delay = spinbutton_reverse_delay_xray_->get_value_as_int();
	reverse.xray_array_delay = spinbutton_reverse_delay_array_->get_value_as_int();
}

Glib::RefPtr<Gtk::ListStore>
AcquisitionParametersDialog::create_step_mode_liststore()
{
	Glib::RefPtr<Gtk::ListStore> liststore = Gtk::ListStore::create(step_mode_model_columns_);

	Gtk::TreeModel::Row row = *(liststore->append());
	row[step_mode_model_columns_.label] = Glib::ustring(_("Half Step"));
	row[step_mode_model_columns_.mode] = HALF_STEP;

	row = *(liststore->append());
	row[step_mode_model_columns_.label] = Glib::ustring(_("Full Step"));
	row[step_mode_model_columns_.mode] = FULL_STEP;

	return liststore;
}

bool
AcquisitionParametersDialog::get_step_mode(const Gtk::ComboBox* combobox)
{
	bool type = 0;
	Gtk::TreeModel::iterator iter = combobox->get_active();
	if (iter) {
		Gtk::TreeRow row = *iter;
		type = static_cast<bool>(row[step_mode_model_columns_.mode]);
	}
	return type;
}

bool
AcquisitionParametersDialog::foreach_step_mode(
	const Gtk::TreeModel::iterator& iter, Gtk::ComboBox* combobox,
	bool mode)
{
	if (iter) {
		Gtk::TreeRow row = *iter;
		if (row[step_mode_model_columns_.mode] == static_cast<StepMode>(mode)) {
			combobox->set_active(iter);
			return true;
		}
	}
	return false;
}

void
AcquisitionParametersDialog::set_step_mode( Gtk::ComboBox* combobox,
	bool mode)
{
	Gtk::TreeModel::SlotForeachIter slot = sigc::bind(
		sigc::mem_fun( *this, &AcquisitionParametersDialog::foreach_step_mode),
		combobox,
		static_cast<StepMode>(mode));

	Glib::RefPtr<Gtk::TreeModel> model = combobox->get_model();
	model->foreach_iter(slot);
}

void
AcquisitionParametersDialog::create_memory_size_liststore()
{
	liststore_memory_size_ = Gtk::ListStore::create(memory_size_model_columns_);

	for ( int i = 1; i <= 4; ++i) {
		Glib::ustring name = ScanAmati::UI::format_value<int>( 8 * i, _("MB"));
		size_t size = i * SCANNER_MEMORY_BANK;

		Gtk::TreeModel::Row row = *(liststore_memory_size_->append());
		row[memory_size_model_columns_.label] = name;
		row[memory_size_model_columns_.size] = size;
	}
}

size_t
AcquisitionParametersDialog::get_memory_size()
{
	size_t size = 0;
	Gtk::TreeModel::iterator iter = combobox_builtin_memory_->get_active();
	if (iter) {
		Gtk::TreeRow row = *iter;
		size = row[memory_size_model_columns_.size];
	}
	return size;
}

bool
AcquisitionParametersDialog::foreach_memory_size(
	const Gtk::TreeModel::iterator& iter, size_t size)
{
	if (iter) {
		Gtk::TreeRow row = *iter;
		if (row[memory_size_model_columns_.size] == size) {
			combobox_builtin_memory_->set_active(iter);
			return true;
		}
	}
	return false;
}

void
AcquisitionParametersDialog::set_memory_size(size_t size)
{
	Gtk::TreeModel::SlotForeachIter slot = sigc::bind(
		sigc::mem_fun( *this, &AcquisitionParametersDialog::foreach_memory_size),
		size);

	Glib::RefPtr<Gtk::TreeModel> model = combobox_builtin_memory_->get_model();
	model->foreach_iter(slot);
}

void
AcquisitionParametersDialog::on_image_width_changed(
	const Gtk::RadioButton* button,
	Scanner::WidthType type)
{
	if (button->get_active()) {
		OFLOG_DEBUG( app.log, "Width type active: " << type);
		width_type_ = type;
	}
}

void
AcquisitionParametersDialog::on_image_calibration_changed(
	const Gtk::RadioButton* button,
	Scanner::CalibrationType type)
{
	if (button->get_active()) {
		OFLOG_DEBUG( app.log, "Calibration type active: " << type);
		calibration_type_ = type;
	}
}

void
AcquisitionParametersDialog::on_image_filter_changed(
	const Gtk::RadioButton* button,
	Magick::FilterTypes type)
{
	if (button->get_active()) {
		OFLOG_DEBUG( app.log, "Filter type active: " << type);
		filter_type_ = type;
	}
}

void
AcquisitionParametersDialog::on_image_intensity_changed(
	const Gtk::RadioButton* button,
	Scanner::PixelIntensityType type)
{
	if (button->get_active()) {
		OFLOG_DEBUG( app.log, "Pixel intensity type active: " << type);
		intensity_type_ = type;
	}
}

void
AcquisitionParametersDialog::on_memory_type_changed(
	const Gtk::RadioButton* button,
	MemoryType type)
{
	size_t size = 0;
	if (button->get_active()) {
		switch (type) {
		case BUILT_IN:
			combobox_builtin_memory_->set_sensitive(true);
			size = get_memory_size();

			spinbutton_accurate_memory_->set_sensitive(false);
			spinbutton_accurate_memory_->set_value(size);

			entry_custom_memory_->set_sensitive(false);
			entry_custom_memory_->set_text(Glib::ustring::format(size));

			break;
		case ACCURATE:
			combobox_builtin_memory_->set_sensitive(false);

			spinbutton_accurate_memory_->set_sensitive(true);
			size = spinbutton_accurate_memory_->get_value_as_int();

			entry_custom_memory_->set_sensitive(false);
			entry_custom_memory_->set_text(Glib::ustring::format(size));
			break;
		case CUSTOM:
			combobox_builtin_memory_->set_sensitive(false);
			spinbutton_accurate_memory_->set_sensitive(false);
			entry_custom_memory_->set_sensitive(true);
			break;
		default:
			break;
		}
	}
}

void
AcquisitionParametersDialog::on_builtin_memory_changed()
{
	size_t size = get_memory_size();
	spinbutton_accurate_memory_->set_value(size);
	entry_custom_memory_->set_text(Glib::ustring::format(size));
}

void
AcquisitionParametersDialog::on_accurate_memory_changed()
{
	size_t size = spinbutton_accurate_memory_->get_value_as_int();
	entry_custom_memory_->set_text(Glib::ustring::format(size));
}

AcquisitionParametersDialog*
AcquisitionParametersDialog::create()
{
	AcquisitionParametersDialog* dialog = 0;
	std::string filename(builder_image_acquisition_parameters_dialog_filename);

	Glib::RefPtr<Gtk::Builder> builder = create_from_file(filename);

	if (builder)
		//Get the GtkBuilder-instantiaited dialog
		builder->get_widget_derived( "dialog-window", dialog);

	return dialog;
}

} // namespace UI

} // namespace ScanAmati
