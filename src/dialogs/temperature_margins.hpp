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

namespace Gtk {
class HScale;
class SpinButton;
} // namespace Gtk

namespace ScanAmati {

namespace UI {

class TemperatureMarginsDialog : public ScannerTemplateDialog {

public:
	static TemperatureMarginsDialog* create();
	TemperatureMarginsDialog( BaseObjectType* cobject,
		const Glib::RefPtr<Gtk::Builder>& builder);
	virtual ~TemperatureMarginsDialog();
	virtual void update_scanner_state(const Scanner::State&);
	virtual void block_interface(bool);
protected:
	// Member functions:
	void init_ui();
	void connect_signals();
	void set_temperature_margins(const Scanner::State&);

	// Signal handlers:
	virtual void on_response(int id);

	Gtk::HScale* hscale_temperature_average_value_;
	Gtk::SpinButton* spinbutton_temperature_spread_value_;
};

} // namespace UI

} // namespace ScanAmati
