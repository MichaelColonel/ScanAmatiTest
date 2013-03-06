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

#pragma once

#include <gtkmm/builder.h>
#include <gtkmm/liststore.h>
#include <gtkmm/dialog.h>

// files from src directory begin
#include "scanner/acquisition.hpp"
// files from src directory end

namespace Gtk {
class Button;
class ComboBox;
class Entry;
class RadioButton;
class SpinButton;
class CheckButton;
class Frame;
} // namespace Gtk

namespace ScanAmati {

namespace UI {

class AcquisitionParametersDialog : public Gtk::Dialog {

public:
	static AcquisitionParametersDialog* create();
	AcquisitionParametersDialog( BaseObjectType* cobject,
		const Glib::RefPtr<Gtk::Builder>& builder);
	virtual ~AcquisitionParametersDialog();
	void set_parameters(const Scanner::AcquisitionParameters& params);
	void get_parameters(Scanner::AcquisitionParameters& params);

	enum StepMode { FULL_STEP, HALF_STEP };
	enum MemoryType { BUILT_IN, ACCURATE, CUSTOM };

protected:
	const struct StepModeModelColumns : public Gtk::TreeModel::ColumnRecord {
		StepModeModelColumns() { add(label); add(mode); }

		Gtk::TreeModelColumn<Glib::ustring> label;
		Gtk::TreeModelColumn<StepMode> mode; // 0 == full step mode, 1 == half step mode
	} step_mode_model_columns_;

	const struct MemorySizeModelColumns : public Gtk::TreeModel::ColumnRecord {
		MemorySizeModelColumns() { add(label); add(size); }

		Gtk::TreeModelColumn<Glib::ustring> label;
		Gtk::TreeModelColumn<size_t> size;
	} memory_size_model_columns_;

	// Member functions:
	void init_ui();
	void connect_signals();

	Glib::RefPtr<Gtk::ListStore> create_step_mode_liststore();
	bool get_step_mode(const Gtk::ComboBox* combobox);
	bool foreach_step_mode( const Gtk::TreeModel::iterator& iter,
		Gtk::ComboBox* combobox, bool mode);
	void set_step_mode( Gtk::ComboBox* combobox, bool mode);

	// Creates only (8, 16, 24, 32 MBytes)
	void create_memory_size_liststore();
	size_t get_memory_size();
	bool foreach_memory_size( const Gtk::TreeModel::iterator& iter, size_t size);
	void set_memory_size(size_t size);

	void set_movements( const Scanner::Movement& forward,
		const Scanner::Movement& reverse);
	void get_movements( Scanner::Movement& forward,
		Scanner::Movement& reverse);

	// Signal handlers:
	virtual void on_response(int id);
	void on_equal_movement_toggled();

	void on_memory_type_changed( const Gtk::RadioButton* button,
		MemoryType);
	void on_image_width_changed( const Gtk::RadioButton* button,
		Scanner::WidthType);
	void on_image_calibration_changed( const Gtk::RadioButton* button,
		Scanner::CalibrationType);
	void on_image_filter_changed( const Gtk::RadioButton* button,
		Magick::FilterTypes);
	void on_image_intensity_changed( const Gtk::RadioButton* button,
		Scanner::PixelIntensityType);

	void on_builtin_memory_changed();
	void on_accurate_memory_changed();

	// Child Widgets:
	Glib::RefPtr<Gtk::Builder> builder_;

	Gtk::CheckButton* checkbutton_array_movement_;
	Gtk::CheckButton* checkbutton_xray_movement_;
	Gtk::CheckButton* checkbutton_exposure_;
	Gtk::CheckButton* checkbutton_acquisition_;

	Gtk::SpinButton* spinbutton_forward_scanning_time_;
	Gtk::SpinButton* spinbutton_forward_steps_;
	Gtk::SpinButton* spinbutton_forward_speed_array_freq_;
	Gtk::SpinButton* spinbutton_forward_speed_xray_freq_;
	Gtk::ComboBox* combobox_forward_speed_array_mode_;
	Gtk::ComboBox* combobox_forward_speed_xray_mode_;
	Gtk::SpinButton* spinbutton_forward_delay_array_;
	Gtk::SpinButton* spinbutton_forward_delay_xray_;

	Gtk::SpinButton* spinbutton_reverse_scanning_time_;
	Gtk::SpinButton* spinbutton_reverse_steps_;
	Gtk::SpinButton* spinbutton_reverse_speed_array_freq_;
	Gtk::SpinButton* spinbutton_reverse_speed_xray_freq_;
	Gtk::ComboBox* combobox_reverse_speed_array_mode_;
	Gtk::ComboBox* combobox_reverse_speed_xray_mode_;
	Gtk::SpinButton* spinbutton_reverse_delay_array_;
	Gtk::SpinButton* spinbutton_reverse_delay_xray_;

	Gtk::CheckButton* checkbutton_equal_movement_;
	Gtk::Frame* frame_reverse_;

	Gtk::RadioButton* radiobutton_width_full_;
	Gtk::RadioButton* radiobutton_width_half_;
	Gtk::RadioButton* radiobutton_width_quarter_;

	Gtk::RadioButton* radiobutton_calibration_rough_;
	Gtk::RadioButton* radiobutton_calibration_good_;
	Gtk::RadioButton* radiobutton_calibration_better_;
	Gtk::RadioButton* radiobutton_calibration_best_;

	Gtk::RadioButton* radiobutton_filter_lanczos_;
	Gtk::RadioButton* radiobutton_filter_cubic_;
	Gtk::RadioButton* radiobutton_filter_bessel_;
	Gtk::RadioButton* radiobutton_filter_quadratic_;

	Gtk::RadioButton* radiobutton_intensity_original_;
	Gtk::RadioButton* radiobutton_intensity_linear_;
	Gtk::RadioButton* radiobutton_intensity_logarithmic_;

	Gtk::RadioButton* radiobutton_memory_builtin_;
	Gtk::RadioButton* radiobutton_memory_accurate_;
	Gtk::RadioButton* radiobutton_memory_custom_;

	Gtk::SpinButton* spinbutton_image_height_;
	Gtk::ComboBox* combobox_builtin_memory_;
	Gtk::SpinButton* spinbutton_accurate_memory_;
	Gtk::Entry* entry_custom_memory_;

	Glib::RefPtr<Gtk::ListStore> liststore_memory_size_;

	Scanner::WidthType width_type_;
	Scanner::CalibrationType calibration_type_;
	Scanner::PixelIntensityType intensity_type_;
	Magick::FilterTypes filter_type_;
};

} // namespace UI

} // namespace ScanAmati
