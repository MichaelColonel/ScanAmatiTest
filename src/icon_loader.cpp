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
#include <gtkmm/icontheme.h>
#include <gtkmm/stock.h>
#include <gtkmm/iconfactory.h>
#include <glibmm/i18n.h>
#include <glib.h> // for G_DIR_SEPARATOR_S

#include <config.h> // for for PACKAGE_NAME

#include "icon_names.hpp"

#include "icon_loader.hpp"

#define ICONS_DIRECTORY \
	SCANAMATI_PKGDATADIR G_DIR_SEPARATOR_S "icons" G_DIR_SEPARATOR_S

namespace {

const char* const file_empty =
	ICONS_DIRECTORY "empty.png";

const char* const file_patient =
	ICONS_DIRECTORY "patient.png";

const char* const file_radiation_active =
	ICONS_DIRECTORY "radiation_indicator_active.png";

const char* const file_radiation_inactive =
	ICONS_DIRECTORY "radiation_indicator_inactive.png";

const char* const file_light_red =
	ICONS_DIRECTORY "light_red.png";

const char* const file_light_yellow =
	ICONS_DIRECTORY "light_yellow.png";

const char* const file_light_green =
	ICONS_DIRECTORY "light_green.png";

const char* const file_light_none =
	ICONS_DIRECTORY "light_none.png";

const struct StockIcons {
	const char* file;
	const char* id;
	const char* label;
} icons[] = {
	{ file_empty, icon_empty, "" },
	{ file_patient, icon_patient, N_("Patient") },
	{ file_radiation_active, icon_radiation_active, N_("X-ray source") },
	{ file_radiation_inactive, icon_radiation_inactive, "" },
	{ file_light_red, icon_light_red, "" },
	{ file_light_yellow, icon_light_yellow, "" },
	{ file_light_green, icon_light_green, "" },
	{ file_light_none, icon_light_none, "" },
	{ } // Terminating entry
};

} // namespace

namespace ScanAmati {

namespace UI {

/** \brief Loads the program icon into a Gdk::Pixbuf.
 *
 * @param size The size of icon; see constants defined in the header.
 * @return A locked and loaded Gdk::Pixbuf.
 * @throw Amati::Exception
 */
Glib::RefPtr<Gdk::Pixbuf>
IconLoader::load_program_icon(int size)
{
	Glib::RefPtr<Gtk::IconTheme> theme = Gtk::IconTheme::get_default();

	if (!theme->has_icon(PACKAGE_NAME)) {
		std::string message(_("Amati icon doesn't seem to be installed "
		"in default icon theme."));
	}

	Glib::RefPtr<Gdk::Pixbuf> icon;

	try {
		icon = theme->load_icon( PACKAGE_NAME, size,
			Gtk::ICON_LOOKUP_USE_BUILTIN);
	}
	catch (const Glib::Error& ex) {
		Glib::ustring message(
			_("Couldn't load program icon from the default icon theme. ") +
			ex.what());
	}

	return icon;
}

/** \brief Registrate stock icons for this program. */
void
IconLoader::register_stock_items()
{
	Glib::RefPtr<Gtk::IconFactory> factory = Gtk::IconFactory::create();

	int i = 0;
	while (icons[i].file) {
		Gtk::IconSource source;

		try {
			// This throws an exception if the file is not found:
			source.set_pixbuf(Gdk::Pixbuf::create_from_file(icons[i].file));
		}
		catch (const Glib::Exception& ex) {
			std::cout << ex.what() << std::endl;
		}

		source.set_size(Gtk::ICON_SIZE_SMALL_TOOLBAR);
		// Icon may be scaled.
		source.set_size_wildcarded();

		Gtk::IconSet icon_set;
		// More than one source per set is allowed.
		icon_set.add_source(source);

		const Gtk::StockID stock_id(icons[i].id);
		factory->add( stock_id, icon_set);
		Gtk::Stock::add(Gtk::StockItem( stock_id, gettext(icons[i].label)));
		++i;
	}

	// Add factory to list of factories:
	factory->add_default();

	// register 64x64 pixels icon size
	Gtk::IconSize::register_new( icon_size_64x64, 64, 64);
}

} // namespace UI

} // namespace ScanAmati
