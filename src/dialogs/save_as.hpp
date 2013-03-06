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

#include <gtkmm/filechooserdialog.h>

namespace Gtk {
class FileFilter;
} // namespace Gtk

namespace ScanAmati {

namespace UI {

class SaveAsDialog : public Gtk::FileChooserDialog {

public:
	SaveAsDialog(Gtk::Window& parent);
	~SaveAsDialog();

protected:
	void init_ui();
	void connect_signals();
	void add_filters();

private:
	Gtk::FileChooserConfirmation on_confirm_overwrite();
	void ok_button_clicked();
	Gtk::FileFilter* filter_dcm_;
	Gtk::FileFilter* filter_raw_;
	Gtk::FileFilter* filter_jpg_;
	Gtk::FileFilter* filter_png_;
	Gtk::Button* button_ok_;
};

} // namespace UI

} // namespace ScanAmati
