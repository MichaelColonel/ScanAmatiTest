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

#include <iostream>
#include <glibmm/i18n.h>
#include <gtkmm/stock.h>
#include <gtkmm/entry.h>

// files from src directory begin
#include "global_strings.hpp"
#include "dicom/utils.hpp"
// files from src directory begin

#include "utils.hpp"
#include "person_name.hpp"

namespace ScanAmati {

namespace UI {

PersonNameDialog::PersonNameDialog( BaseObjectType* cobject,
	const Glib::RefPtr<Gtk::Builder>& builder)
	:
	Gtk::Dialog(cobject),
	builder_(builder)
{
	init_ui();

	connect_signals();

	show_all_children();
}

PersonNameDialog::~PersonNameDialog()
{
}

void
PersonNameDialog::init_ui()
{
	builder_->get_widget( "entry-last-name", entries_[0]);
	builder_->get_widget( "entry-first-name", entries_[1]);
	builder_->get_widget( "entry-middle-name", entries_[2]);
	builder_->get_widget( "entry-prefix", entries_[3]);
	builder_->get_widget( "entry-suffix", entries_[4]);
}

void
PersonNameDialog::connect_signals()
{
}

void
PersonNameDialog::on_response(int res)
{
	switch (res) {
	case 0:
		break;
	case 1:
		{
			Glib::ustring name = format_name();
			signal_name_changed_(name);
		}
		break;
	default:
		break;
	}

	hide();
}

Glib::ustring
PersonNameDialog::format_name() const
{
	Glib::ustring comp[PERSON_NAME_COMPONENTS];
	const Glib::ustring& last = comp[0];
	const Glib::ustring& first = comp[1];
	const Glib::ustring& middle = comp[2];
	const Glib::ustring& prefix = comp[3];
	const Glib::ustring& suffix = comp[4];

	for ( int i = 0; i < PERSON_NAME_COMPONENTS; ++i)
		comp[i] = entries_[i]->get_text();

	return DICOM::name_components_to_dicom( last, first, middle,
		prefix, suffix);
}

void
PersonNameDialog::fill_name_components(const Glib::ustring& name)
{
	Glib::ustring comp[PERSON_NAME_COMPONENTS];
	Glib::ustring& last = comp[0];
	Glib::ustring& first = comp[1];
	Glib::ustring& middle = comp[2];
	Glib::ustring& prefix = comp[3];
	Glib::ustring& suffix = comp[4];

	if (DICOM::dicom_to_name_components( name, last, first,
		middle, prefix, suffix)) {
		for ( int i = 0; i < PERSON_NAME_COMPONENTS; ++i)
			entries_[i]->set_text(comp[i]);
	}
}

sigc::signal< void, const Glib::ustring&>
PersonNameDialog::signal_name_changed()
{
	return signal_name_changed_;
}

PersonNameDialog*
PersonNameDialog::create(const Glib::ustring& name)
{
	PersonNameDialog* dialog = 0;
	std::string filename(builder_person_name_dialog_filename);

	Glib::RefPtr<Gtk::Builder> builder = create_from_file(filename);

	if (builder)
		//Get the GtkBuilder-instantiated dialog
		builder->get_widget_derived( "dialog-window", dialog);

	if (!name.empty())
		dialog->fill_name_components(name);

	return dialog;
}

} // namespace UI

} // namespace ScanAmati
