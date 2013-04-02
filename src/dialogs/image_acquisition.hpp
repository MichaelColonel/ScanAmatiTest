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

#include <gtkmm/liststore.h>

// files from src directory begin
#include "scanner/acquisition.hpp"
#include "scanner/data.hpp"
// files from src directory end

#include "scanner_template.hpp"

namespace Gtk {
class Adjustment;
class HScale;
class Button;
class Image;
class ComboBox;
class RadioButton;
class CheckButton;
class Entry;
class Label;
} // namespace Gtk

namespace ScanAmati {

namespace UI {

class ImageAcquisitionDialog : public ScannerTemplateDialog {

public:
	static ImageAcquisitionDialog* create();
	ImageAcquisitionDialog( BaseObjectType* cobject,
		const Glib::RefPtr<Gtk::Builder>& builder);
	virtual ~ImageAcquisitionDialog();
	virtual void update_scanner_state(const Scanner::State&);

protected:
	// Member functions:
	void init_ui();
	void connect_signals();

	// Signal handlers:
	virtual void on_response(int id);
	void on_xray_parameters_changed();
	void on_run_arguments_changed();
	void on_acquisition_run();
	void on_acquisition_stop();
	void on_acquisition_parameters();

	Glib::ustring on_format_exposure_value(double exposure);
	Glib::ustring on_format_voltage_value(double voltage);

	void get_parameters(Scanner::AcquisitionParameters&);
	void set_parameters(const Scanner::AcquisitionParameters&);

	// Child widgets:
	Gtk::HScale* scale_xray_voltage_;
	Glib::RefPtr<Gtk::Adjustment> adjustment_xray_voltage_;

	Gtk::HScale* scale_xray_exposure_;
	Glib::RefPtr<Gtk::Adjustment> adjustment_xray_exposure_;

	Gtk::Button* button_acquisition_run_;
	Gtk::Button* button_acquisition_stop_;

	Gtk::Image* image_radiation_indicator_;

	Gtk::CheckButton* checkbutton_array_movement_;
	Gtk::CheckButton* checkbutton_xray_movement_;
	Gtk::CheckButton* checkbutton_exposure_;
	Gtk::CheckButton* checkbutton_acquisition_;

	Gtk::RadioButton* radiobutton_memory_8mbytes_;
	Gtk::RadioButton* radiobutton_memory_16mbytes_;
	Gtk::RadioButton* radiobutton_memory_24mbytes_;
	Gtk::RadioButton* radiobutton_memory_32mbytes_;

	Gtk::Button* button_acquisition_parameters_;

private:
	// Members:
	std::vector<double> exposures_;
	Scanner::AcquisitionParameters acquisition_;
};

} // namespace UI

} // namespace ScanAmati
