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

#include <gtkmm/dialog.h>
#include <gtkmm/builder.h>

namespace Gtk {
class Entry;
} // namespace Gtk

namespace ScanAmati {

namespace UI {

class PersonNameDialog : public Gtk::Dialog {
public:
	static PersonNameDialog* create(const Glib::ustring& name = "");
	PersonNameDialog( BaseObjectType* cobject,
		const Glib::RefPtr<Gtk::Builder>& builder);
	virtual ~PersonNameDialog();

	sigc::signal< void, const Glib::ustring&> signal_name_changed();

protected:
	enum { PERSON_NAME_COMPONENTS = 5 };

	// UI methods:
	void init_ui();
	void connect_signals();
	virtual void on_response(int);
	Glib::ustring format_name() const;
	void fill_name_components(const Glib::ustring&);

	// Members
	Glib::RefPtr<Gtk::Builder> builder_;
	Gtk::Entry* entries_[PERSON_NAME_COMPONENTS];
	// Signals
	sigc::signal< void, const Glib::ustring&> signal_name_changed_;
};

} // namespace UI

} // namespace ScanAmati
