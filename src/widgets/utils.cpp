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

#include <glibmm/i18n.h>

#include "utils.hpp"

namespace ScanAmati {

namespace UI {

void
add_row( Gtk::Table& table, int row,
	const Glib::ustring& label_text, Gtk::Widget &widget,
	Glib::RefPtr<Gtk::SizeGroup>& size_group)
{
	Gtk::Label* label = Gtk::manage(new Gtk::Label( label_text, true));
	label->set_alignment(Gtk::ALIGN_RIGHT);

	table.attach( *label, 0, 1, row, row + 1, Gtk::SHRINK | Gtk::FILL,
		Gtk::AttachOptions(0));

	label->set_mnemonic_widget(widget);

	size_group->add_widget(widget);

	table.attach( widget, 1, 2, row, row + 1, Gtk::EXPAND | Gtk::FILL,
		Gtk::AttachOptions(0));	
}

void
add_row_with_label_markup( Gtk::Table& table, int row,
	const Glib::ustring& label_text, Gtk::Widget &widget,
	Glib::RefPtr<Gtk::SizeGroup>& size_group)
{
	Gtk::Label* label = Gtk::manage(new Gtk::Label);
	label->set_markup_with_mnemonic(label_text);
	label->set_alignment(Gtk::ALIGN_RIGHT);

	table.attach( *label, 0, 1, row, row + 1, Gtk::SHRINK | Gtk::FILL,
		Gtk::AttachOptions(0));

	label->set_mnemonic_widget(widget);

	size_group->add_widget(widget);

	table.attach( widget, 1, 2, row, row + 1, Gtk::EXPAND | Gtk::FILL,
		Gtk::AttachOptions(0));	
}

void
add_row( Gtk::Table& table, int row,
	const Glib::ustring& label_text, Gtk::Widget &widget,
	Glib::RefPtr<Gtk::SizeGroup>& size_group,
	const Glib::ustring& suffix_text)
{
	Gtk::Label* label = Gtk::manage(new Gtk::Label( label_text, true));
	label->set_alignment(Gtk::ALIGN_RIGHT);

	table.attach( *label, 0, 1, row, row + 1, Gtk::SHRINK | Gtk::FILL,
		Gtk::AttachOptions(0));

	label->set_mnemonic_widget(widget);

	Gtk::Label* label_suffix = Gtk::manage(new Gtk::Label(suffix_text));
	Gtk::Box* box = Gtk::manage(new Gtk::HBox);
	box->set_spacing(5);
	box->pack_start( widget, Gtk::PACK_SHRINK);
	box->pack_start( *label_suffix, Gtk::PACK_SHRINK);

	size_group->add_widget(widget);

	table.attach( *box, 1, 2, row, row + 1, Gtk::SHRINK | Gtk::FILL,
		Gtk::AttachOptions(0));
}

void
add_row_reverse( Gtk::Table& table, int row,
	const Glib::ustring& label_text, Gtk::Widget &widget)
{
	table.attach( widget, 0, 1, row, row + 1, Gtk::SHRINK | Gtk::FILL,
		Gtk::AttachOptions(0));

	Gtk::Label* label = Gtk::manage(new Gtk::Label(label_text));
	label->set_alignment(Gtk::ALIGN_LEFT);

	table.attach( *label, 1, 2, row, row + 1, Gtk::EXPAND | Gtk::FILL,
		Gtk::AttachOptions(0));
}

void
add_row_reverse( Gtk::Table& table, int row,
	Gtk::Label& label_widget, Gtk::Widget &widget)
{
	table.attach( widget, 0, 1, row, row + 1, Gtk::SHRINK | Gtk::FILL,
		Gtk::AttachOptions(0));

	label_widget.set_alignment(Gtk::ALIGN_LEFT);

	table.attach( label_widget, 1, 2, row, row + 1, Gtk::EXPAND | Gtk::FILL,
		Gtk::AttachOptions(1));
}

Gtk::ComboBoxEntryText*
create_combobox_entry_text(const std::list<Glib::ustring>& strings)
{
	Gtk::ComboBoxEntryText* combo = Gtk::manage(new Gtk::ComboBoxEntryText());

	for ( std::list<Glib::ustring>::const_iterator iter = strings.begin();
		iter != strings.end(); ++iter)
		combo->append_text(*iter);

	return combo;
}

Gtk::ComboBoxText*
create_combobox_text(const std::list<Glib::ustring>& strings)
{
	Gtk::ComboBoxText* combo = Gtk::manage(new Gtk::ComboBoxText());

	for ( std::list<Glib::ustring>::const_iterator iter = strings.begin();
		iter != strings.end(); ++iter)
		combo->append_text(*iter);

	return combo;
}

Glib::ustring
format_temperature_value(double value)
{
	Glib::ustring sign;

	if (value > 0.0)
		sign = "+"; // plus
	else if (value < 0.0)
		sign = "\u2212"; // minus

	return Glib::ustring::compose( "%1%2 \u2103", sign,
		Glib::ustring::format( std::setprecision(1), std::fixed, fabs(value)));
}

Glib::ustring
format_temperature_margins( double average, double spread)
{
	return Glib::ustring::compose( "(%1 \u00B1 %2) \u2103",
		Glib::ustring::format( std::showpos,
			std::setprecision(1), std::fixed, average),
		Glib::ustring::format( std::setprecision(1), std::fixed, spread));
}

Glib::ustring
format_height_value(double value)
{
	Glib::ustring text = Glib::ustring::compose( "%1 %2",
		Glib::ustring::format(value), Glib::ustring(_("cm")));
	return text;
}

Glib::ustring
format_weight_value(double value)
{
	Glib::ustring text = Glib::ustring::compose( "%1 %2",
		Glib::ustring::format(value), Glib::ustring(_("kg")));
	return text;
}

} // namespace UI

} // namespace ScanAmati
