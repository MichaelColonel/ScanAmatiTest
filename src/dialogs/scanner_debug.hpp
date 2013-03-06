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

#include "scanner_template.hpp"

// files from src directory begin
#include "scanner/commands.hpp"
// files from src directory end

namespace Gtk {
class Button;
class CheckButton;
}

namespace ScanAmati {

namespace UI {

class ScannerDebugDialog : public ScannerTemplateDialog {

public:
	static ScannerDebugDialog* create();
	ScannerDebugDialog( BaseObjectType* cobject,
		const Glib::RefPtr<Gtk::Builder>& builder);
	virtual ~ScannerDebugDialog();
	virtual void update_scanner_state(const Scanner::State&);

protected:
	// Member functions:
	void connect_signals();
	void init_ui();
	void set_temperature_control(const Scanner::State&);

	// Signal handlers:
	virtual void on_response(int res);

	void on_scanner_command(Scanner::CommandType);
	void on_write_lining();
	void on_temperature_control();
	void on_adjust_lining();
	void on_select_capacity();

	// Widgets:
	Gtk::Button* button_altera_reset_;
	Gtk::Button* button_array_reset_;
	Gtk::Button* button_write_lining_;
	Gtk::Button* button_adjust_lining_;
	Gtk::Button* button_select_capacity_;
	Gtk::Button* button_turnoff_peltier_;
	Gtk::CheckButton* checkbutton_temperature_control_;

	// Signals:
	sigc::signal< void, const Scanner::State&> signal_scanner_state_changed_;
};

} // namespace UI

} // namespace ScanAmati
