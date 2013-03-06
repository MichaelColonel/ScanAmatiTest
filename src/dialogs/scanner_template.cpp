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

// files from src directory begin
#include "global_strings.hpp"
#include "application.hpp"
// files from src directory end

#include "scanner_template.hpp"

namespace ScanAmati {

namespace UI {

ScannerTemplateDialog::ScannerTemplateDialog( BaseObjectType* cobject,
		const Glib::RefPtr<Gtk::Builder>& builder,
		const Glib::ustring& keyname)
	:
	Gtk::Dialog(cobject),
	builder_(builder),
	keyname_(keyname),
	ui_state_initiated_(false)
{
	int h = app.prefs.get<int>( "Gui", keyname_ + conf_key_dialog_height);
	int w = app.prefs.get<int>( "Gui", keyname_ + conf_key_dialog_width);
	resize( w, h);
}

ScannerTemplateDialog::~ScannerTemplateDialog()
{
	int w, h;

	get_size( w, h);

	app.prefs.set( "Gui", keyname_ + conf_key_dialog_height, h);
	app.prefs.set( "Gui", keyname_ + conf_key_dialog_width, w);
}

} // namespace UI

} // namespace ScanAmati
