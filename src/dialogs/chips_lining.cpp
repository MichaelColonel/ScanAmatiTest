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
#include <iomanip>
#include <gtkmm/treeview.h>
#include <gtkmm/cellrenderer.h>
#include <gtkmm/treemodelcolumn.h>

// files from src directory begin
#include "scanner/data.hpp"
#include "global_strings.hpp"
// files from src directory end

#include "utils.hpp"
#include "chips_lining.hpp"

namespace {

const struct ModelColumns : public Gtk::TreeModelColumnRecord {
	Gtk::TreeModelColumn<Glib::ustring> label;
	Gtk::TreeModelColumn<Glib::ustring> label_code;
	Gtk::TreeModelColumn<gchar> code;

	ModelColumns() { add(label); add(label_code); add(code); }
} model_columns;

} // namespace

namespace ScanAmati {

namespace UI {

ChipsLiningDialog::ChipsLiningDialog( BaseObjectType* cobject,
	const Glib::RefPtr<Gtk::Builder>& builder)
	:
	ScannerTemplateDialog( cobject, builder, "chips-lining"),
	builder_(builder),
	treeview_chips_(0)
{
	init_ui();

	connect_signals();

	show_all_children();
}

ChipsLiningDialog::~ChipsLiningDialog()
{
}

void
ChipsLiningDialog::init_ui()
{
	Glib::RefPtr<Glib::Object> obj;
	obj = builder_->get_object("liststore-chips");
	liststore_chips_ = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(obj);

	// add rows
	for ( int i = 0; i < SCANNER_CHIPS; ++i) {
		char code = Scanner::Data::chip_code(i);

		Gtk::TreeRow row = *(liststore_chips_->append());
		row[model_columns.label] = Glib::ustring::format(
			std::setfill(L' '), std::setw(2), i + 1);
		row[model_columns.label_code] = Glib::ustring( 1, code);
		row[model_columns.code] = code;
	}

	builder_->get_widget( "treeview-chips", treeview_chips_);

	selection_chips_ = treeview_chips_->get_selection();
	selection_chips_->set_mode(Gtk::SELECTION_MULTIPLE);

	// add columns
	Gtk::TreeViewColumn* column;
	Gtk::CellRenderer* renderer;
	column = new Gtk::TreeView::Column( _("Chip"), model_columns.label);
	column->set_alignment(Gtk::ALIGN_CENTER);
	renderer = column->get_first_cell_renderer();
	renderer->property_xalign() = .5;
	treeview_chips_->append_column(*Gtk::manage(column));

	column = new Gtk::TreeView::Column( _("Code"), model_columns.label_code);
	column->set_alignment(Gtk::ALIGN_CENTER);
	renderer = column->get_first_cell_renderer();
	renderer->property_xalign() = .5;
	treeview_chips_->append_column(*Gtk::manage(column));
}

void
ChipsLiningDialog::connect_signals()
{
}

void
ChipsLiningDialog::update_scanner_state(const Scanner::State&)
{
}

void
ChipsLiningDialog::block_interface(bool)
{
}

void
ChipsLiningDialog::set_lining_chips(const std::vector<char>& chips)
{
	for ( std::vector<char>::const_iterator it = chips.begin();
		it != chips.end(); ++it) {
			Glib::ustring text = Glib::ustring::format(Scanner::Data::chip_number(*it));
			Gtk::TreeIter iter = liststore_chips_->get_iter(text);
			if (iter)
				selection_chips_->select(iter);
		}
}

void
ChipsLiningDialog::on_response(int)
{
	hide();
}

ChipsLiningDialog*
ChipsLiningDialog::create(const std::vector<char>& chips)
{
	ChipsLiningDialog* dialog = 0;
	std::string filename(builder_lining_acquisition_dialog_filename);

	Glib::RefPtr<Gtk::Builder> builder = create_from_file(filename);

	if (builder)
		//Get the GtkBuilder-instantiated dialog
		builder->get_widget_derived( "dialog-chips", dialog);

	if (dialog)
		dialog->set_lining_chips(chips);

	return dialog;
}

} // namespace UI

} // namespace ScanAmati
