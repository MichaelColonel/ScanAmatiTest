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
class SpinButton;
class Button;
class MenuToolButton;
class MenuItem;
class ComboBox;
} // namespace Gtk

namespace ScanAmati {

namespace UI {

class LiningAdjustmentDialog : public ScannerTemplateDialog {
public:
	static LiningAdjustmentDialog* create();
	LiningAdjustmentDialog( BaseObjectType* cobject,
		const Glib::RefPtr<Gtk::Builder>& builder);
	virtual ~LiningAdjustmentDialog();
	virtual void update_scanner_state(const Scanner::State&);
protected:
	// Member functions:
	void connect_signals();
	void init_ui();
	void set_lining_data(const Scanner::State&);
	void set_adjustment_sensitive(bool state);
	void set_strip_code( guint strip, guint8 code);

	// signal handlers:
	virtual void on_response(int);
	void on_chip_changed();
	void on_strip_changed();
	void on_code_changed();
	void on_broadcast_changed();
	void on_selection_changed();
	void on_load_lining();
	void on_write_lining();
	void on_write_all_lining();
	Gtk::TreeView* treeview_strip_code_;
	Gtk::ComboBox* combobox_chip_;
	Gtk::SpinButton* spinbutton_strip_;
	Gtk::SpinButton* spinbutton_code_;
	Gtk::SpinButton* spinbutton_broadcast_;

	Gtk::MenuToolButton* menutoolbutton_write_current_;
	Gtk::MenuItem* menuitem_write_all_;

	Gtk::MenuToolButton* menutoolbutton_load_from_file_;
	Gtk::MenuItem* menuitem_restore_current_;
	Gtk::MenuItem* menuitem_restore_all_;

	Glib::RefPtr<Gtk::ListStore> liststore_strip_code_;
	Glib::RefPtr<Gtk::TreeSelection> selection_strip_code_;
	std::map< char, std::vector<guint8> > lining_;
};

} // namespace UI

} // namespace ScanAmati
