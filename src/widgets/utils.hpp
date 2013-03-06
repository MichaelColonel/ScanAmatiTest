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

#include <gtkmm/box.h>
#include <gtkmm/table.h>
#include <gtkmm/widget.h>
#include <gtkmm/label.h>
#include <gtkmm/sizegroup.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/comboboxentrytext.h>

#include <iomanip>

#include <list>

namespace ScanAmati {

namespace UI {

void
add_row( Gtk::Table& table, int row, const Glib::ustring& label_text,
	Gtk::Widget &widget, Glib::RefPtr<Gtk::SizeGroup>& size_group);

void
add_row_with_label_markup( Gtk::Table& table, int row,
	const Glib::ustring& label_text, Gtk::Widget &widget,
	Glib::RefPtr<Gtk::SizeGroup>& size_group);

void
add_row( Gtk::Table& table, int row, const Glib::ustring& label_text,
	Gtk::Widget &widget, Glib::RefPtr<Gtk::SizeGroup>& size_group,
	const Glib::ustring& suffix_text);

void
add_row_reverse( Gtk::Table& table, int row, const Glib::ustring& label_text,
	Gtk::Widget &widget);

void
add_row_reverse( Gtk::Table& table, int row, Gtk::Label& label_widget,
	Gtk::Widget &widget);

Gtk::ComboBoxEntryText*
create_combobox_entry_text(const std::list<Glib::ustring>& strings);

Gtk::ComboBoxText*
create_combobox_text(const std::list<Glib::ustring>& strings);

Glib::ustring
format_temperature_value(double temperature);

Glib::ustring
format_temperature_margins( double average, double spread);

Glib::ustring
format_weight_value(double weight);

Glib::ustring
format_height_value(double height);

template<typename T>
Glib::ustring
format_value( T value, const Glib::ustring& suffix)
{
	Glib::ustring text = Glib::ustring::compose( "%1 %2",
		Glib::ustring::format( std::setfill(L' '), std::setw(3), value),
		suffix);

	return text;
}

template<typename T>
Glib::ustring
format_precision_value( T value, const Glib::ustring& suffix, int precision)
{
	Glib::ustring text = Glib::ustring::compose( "%1 %2",
		Glib::ustring::format( std::setfill(L' '), std::setw(2), 
		std::setprecision(precision), std::fixed, value),
		suffix);

	return text;
}

} // namespace UI

} // namespace ScanAmati
