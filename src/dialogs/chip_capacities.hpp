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

#include <map>
#include "scanner_template.hpp"

namespace Gtk {
class Box;
class RadioButton;
}

namespace ScanAmati {

namespace UI {

class ChipCapacitiesDialog : public ScannerTemplateDialog {

public:
	static ChipCapacitiesDialog* create();
	ChipCapacitiesDialog( BaseObjectType* cobject,
		const Glib::RefPtr<Gtk::Builder>& builder);
	virtual ~ChipCapacitiesDialog();
	virtual void update_scanner_state(const Scanner::State&);
	virtual void block_interface(bool);
protected:
	// Member functions:
	void connect_signals();
	void init_ui();
	void set_current_capacity(const Scanner::State&);

	void on_capacity_changed(double capacity);
	void insert_into_buttons_map( double capacity, Gtk::RadioButton* button);
	virtual void on_response(int res);

	std::map< double, Gtk::RadioButton*> buttons_map_;

	double capacity_;
};

} // namespace UI

} // namespace ScanAmati
