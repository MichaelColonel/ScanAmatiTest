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

namespace ScanAmati {

namespace Scanner {
class State;
} // namespace Scanner

namespace UI {

class ScannerTemplateDialog : public Gtk::Dialog {
public:
	ScannerTemplateDialog( BaseObjectType* cobject,
		const Glib::RefPtr<Gtk::Builder>& builder,
		const Glib::ustring& keyname);
	virtual ~ScannerTemplateDialog();
	virtual void update_scanner_state(const Scanner::State&) = 0;
protected:
	Glib::RefPtr<Gtk::Builder> builder_;
	Glib::ustring keyname_;
	bool ui_state_initiated_;
};

} // namespace UI

} // namespace ScanAmati
