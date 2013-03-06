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

#include <gtkmm/stock.h>
#include <gtkmm/filefilter.h>
#include <gtkmm/messagedialog.h>
#include <gdkmm/pixbufformat.h>
#include <glibmm/i18n.h>

// files from src directory begin
#include "global_strings.hpp"
#include "application.hpp"
// files from src directory end

#include "save_as.hpp"

namespace ScanAmati {

namespace UI {

SaveAsDialog::SaveAsDialog(Gtk::Window& parent) : Gtk::FileChooserDialog(
	parent, _("Save File"), Gtk::FILE_CHOOSER_ACTION_SAVE),
	filter_dcm_(0),
	filter_raw_(0),
	filter_jpg_(0),
	filter_png_(0),
	button_ok_(0)
{
	init_ui();
	connect_signals();

	show_all_children();
}

SaveAsDialog::~SaveAsDialog()
{
}

void
SaveAsDialog::init_ui()
{
	set_do_overwrite_confirmation(true);

	add_filters();

	set_filter(*filter_dcm_);

	// Add response buttons in the dialog:
	add_button( Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	button_ok_ = add_button( Gtk::Stock::SAVE, Gtk::RESPONSE_OK);
}

void
SaveAsDialog::connect_signals()
{
	signal_confirm_overwrite().connect(
		sigc::mem_fun( *this, &SaveAsDialog::on_confirm_overwrite));

	button_ok_->signal_clicked().connect(
		sigc::mem_fun( *this, &SaveAsDialog::ok_button_clicked));
}

Gtk::FileChooserConfirmation
SaveAsDialog::on_confirm_overwrite()
{
	Glib::ustring what = Glib::ustring::compose(
		_("The file \"%1\" already exists.\nDo you want to replace it?"),
		Glib::filename_to_utf8(Glib::path_get_basename(get_filename())));

	Glib::ustring todo = Glib::ustring::compose(
		_("The file with the same name already exists "
		"in the folder \"%1\".\nReplacing it will overwrite "
		"its current contents."),
		Glib::path_get_basename(get_current_folder()));

	Gtk::MessageDialog dialog( *this, "", false, Gtk::MESSAGE_WARNING,
		Gtk::BUTTONS_NONE);

	dialog.set_message(what);
	dialog.set_secondary_text(todo);
	dialog.property_secondary_use_markup() = true;
	dialog.add_button( _("Cancel"), Gtk::RESPONSE_NO);
	dialog.add_button( _("Replace"), Gtk::RESPONSE_YES);


	int res = dialog.run();

	Gtk::FileChooserConfirmation conf;
	switch (res) {
	case Gtk::RESPONSE_YES:
		conf = Gtk::FILE_CHOOSER_CONFIRMATION_ACCEPT_FILENAME;
		break;
	case Gtk::RESPONSE_NO:
	default:
		conf = Gtk::FILE_CHOOSER_CONFIRMATION_SELECT_AGAIN;
		break;
	}
	return conf;
}

void
SaveAsDialog::ok_button_clicked()
{
}

void
SaveAsDialog::add_filters()
{
	// Add filters, so that only certain file types can be selected:
	filter_dcm_ = Gtk::manage(new Gtk::FileFilter());
	filter_dcm_->set_name(gettext(filter_dcm_name));
	filter_dcm_->add_mime_type("application/dicom");
	add_filter(*filter_dcm_);

	filter_raw_ = Gtk::manage(new Gtk::FileFilter());
	filter_raw_->set_name(gettext(filter_raw_name));
	filter_raw_->add_pattern("*.raw");

	if (app.extend) {
		add_filter(*filter_raw_);
	}

	filter_jpg_ = Gtk::manage(new Gtk::FileFilter());
	filter_jpg_->set_name(gettext(filter_jpg_name));
	filter_jpg_->add_pattern("*.jpg");
	filter_jpg_->add_pattern("*.jpeg");
	add_filter(*filter_jpg_);

	filter_png_ = Gtk::manage(new Gtk::FileFilter());
	filter_png_->set_name(gettext(filter_png_name));
	filter_png_->add_pattern("*.png");
	add_filter(*filter_png_);
}

} // namespace UI

} // namespace ScanAmati
