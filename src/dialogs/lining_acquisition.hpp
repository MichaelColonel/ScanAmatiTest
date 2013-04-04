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

// files from src directory begin
#include "scanner/data.hpp"
// files from src directory end

#include "scanner_template.hpp"

namespace Gtk {
class Button;
class SpinButton;
class RadioButton;
class Expander;
} // namespace Gtk

namespace ScanAmati {

namespace UI {

class LiningAcquisitionDialog : public ScannerTemplateDialog {

public:
	static LiningAcquisitionDialog* create();
	LiningAcquisitionDialog( BaseObjectType* cobject,
		const Glib::RefPtr<Gtk::Builder>& builder);
	virtual ~LiningAcquisitionDialog();
	virtual void update_scanner_state(const Scanner::State&);
	virtual void block_interface(bool);

protected:
	// Member functions:
	void init_ui();
	void connect_signals();

	// Signal handlers:
	virtual void on_response(int id);
	void on_start();
	void on_stop();
	void on_accuracy_changed(Scanner::LiningAccuracyType);

	Gtk::SpinButton* spinbutton_adc_count_;
	Gtk::Button* button_start_;
	Gtk::Button* button_stop_;

	Gtk::Expander* expander_accuracy_;
	Gtk::RadioButton* radiobutton_accuracy_rough_;
	Gtk::RadioButton* radiobutton_accuracy_optimal_;
	Gtk::RadioButton* radiobutton_accuracy_precise_;

	Scanner::LiningAccuracyType accuracy_;

	// Signals:
	sigc::signal<void> signal_lining_ready_;
};

} // namespace UI

} // namespace ScanAmati
