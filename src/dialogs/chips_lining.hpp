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
#include <gtkmm/treeselection.h>

#include "scanner_template.hpp"

namespace Gtk {
class TreeView;
} // namespace Gtk

namespace ScanAmati {

namespace UI {

class ChipsLiningDialog : public ScannerTemplateDialog {
public:
	static ChipsLiningDialog* create(const std::vector<char>&);
	ChipsLiningDialog( BaseObjectType* cobject,
		const Glib::RefPtr<Gtk::Builder>& builder);
	virtual ~ChipsLiningDialog();
	virtual void update_scanner_state(const Scanner::State&);
	virtual void block_interface(bool);
	Glib::RefPtr<Gtk::TreeSelection> selection_chips() { return selection_chips_; }

protected:
	// Member functions:
	void connect_signals();
	void init_ui();
	void set_lining_chips(const std::vector<char>&);

	// signal handlers:
	virtual void on_response(int);

	Gtk::TreeView* treeview_chips_;

	Glib::RefPtr<Gtk::Builder> builder_;
	Glib::RefPtr<Gtk::ListStore> liststore_chips_;
	Glib::RefPtr<Gtk::TreeSelection> selection_chips_;
};

} // namespace UI

} // namespace ScanAmati
