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

#include <gtkmm/builder.h>
#include <gtkmm/messagedialog.h>

namespace ScanAmati {

namespace UI {

class InfoDialog : public Gtk::MessageDialog {
public:
	InfoDialog( Gtk::Window& parent, const Glib::ustring& title)
	:
	Gtk::MessageDialog(
		parent,
		title,
		true,
		Gtk::MESSAGE_INFO,
		Gtk::BUTTONS_CLOSE)
	{
	}
};

class ErrorDialog : public Gtk::MessageDialog {
public:
	ErrorDialog( Gtk::Window& parent, const Glib::ustring& title)
	:
	Gtk::MessageDialog(
		parent,
		title,
		true,
		Gtk::MESSAGE_ERROR,
		Gtk::BUTTONS_CLOSE)
	{
	}
};

class WarningDialog : public Gtk::MessageDialog {
public:
	WarningDialog( Gtk::Window& parent, const Glib::ustring& title)
	:
	Gtk::MessageDialog(
		parent,
		title,
		true,
		Gtk::MESSAGE_WARNING,
		Gtk::BUTTONS_CLOSE)
	{
	}
};

Glib::RefPtr<Gtk::Builder> create_from_file(const std::string& filename);

} // namespace UI

} // namespace ScanAmati
