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
#include <gtkmm/pagesetup.h>
#include <gtkmm/printsettings.h>

#include "print/operation.hpp"

namespace Gtk {
class Button;
} // namespace Gtk

namespace ScanAmati {

namespace UI {

class PrintParametersDialog : public Gtk::Dialog {
public:
	static PrintParametersDialog* create();
	PrintParametersDialog( BaseObjectType* cobject,
		const Glib::RefPtr<Gtk::Builder>& builder);
	virtual ~PrintParametersDialog();

protected:
	// Member functions:
	void connect_signals();
	void init_ui();

	// signal handlers:
	virtual void on_response(int);

	void on_printoperation_status_changed(
		const Glib::RefPtr<Print::Operation>& operation);
	void on_printoperation_done( Gtk::PrintOperationResult result,
		const Glib::RefPtr<Print::Operation>& operation);

	Glib::RefPtr<Gtk::Builder> builder_;
	Glib::RefPtr<Gtk::PageSetup> page_setup_;
	Glib::RefPtr<Gtk::PrintSettings> print_settings_;
};

} // namespace UI

} // namespace ScanAmati
