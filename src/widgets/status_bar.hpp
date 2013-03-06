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

#include <gtkmm/statusbar.h>
#include <gtkmm/builder.h>

namespace Gtk {
class ProgressBar;
} // namespace Gtk

namespace ScanAmati {

namespace Scanner {
class State;
} // namespace Scanner

namespace UI {

class Statusbar : public Gtk::Statusbar {

public:
	Statusbar( BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>&);
	virtual ~Statusbar();
	void set_text(const Glib::ustring&);
	void set_text_timed(const Glib::ustring&);
	void set_progress(double);
	void set_pulse_progress();
	void update_scanner_state(const Scanner::State&);

protected:
	void init_ui();
	void connect_signals();

	Glib::RefPtr<Gtk::Builder> builder_;
	Gtk::ProgressBar* progressbar_;
};

} // namespace UI

} // namespace ScanAmati
