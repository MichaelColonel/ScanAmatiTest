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

#include <gtkmm/treeview.h>
#include <gtkmm/cellrenderer.h>
#include <gtkmm/treemodelcolumn.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/menutoolbutton.h>
#include <gtkmm/combobox.h>

// files from src directory begin
#include "scanner/manager.hpp"
#include "global_strings.hpp"
// files from src directory end

#include "utils.hpp"
#include "lining_adjustment.hpp"

namespace {

typedef std::map< char, std::vector<guint8> >::const_iterator LiningConstIter;
typedef std::pair< guint, guint> StripCode;
typedef std::vector<StripCode> StripCodeVector;

const struct ModelColumns : public Gtk::TreeModelColumnRecord {
	Gtk::TreeModelColumn<guint> strip;
	Gtk::TreeModelColumn<guint> code;

	ModelColumns() { add(strip); add(code); }
} model_columns;

} // namespace

namespace ScanAmati {

namespace UI {

LiningAdjustmentDialog::LiningAdjustmentDialog( BaseObjectType* cobject,
	const Glib::RefPtr<Gtk::Builder>& builder)
	:
	ScannerTemplateDialog( cobject, builder, "lining-adjustment"),
	treeview_strip_code_(0),
	spinbutton_strip_(0),
	spinbutton_code_(0),
	spinbutton_broadcast_(0),
	menutoolbutton_write_current_(0),
	menuitem_write_all_(0),
	menutoolbutton_load_from_file_(0),
	menuitem_restore_current_(0),
	menuitem_restore_all_(0)
{
	init_ui();

	connect_signals();

	set_adjustment_sensitive(false);

	show_all_children();
}

LiningAdjustmentDialog::~LiningAdjustmentDialog()
{
}

void
LiningAdjustmentDialog::init_ui()
{
	builder_->get_widget( "combobox-chip", combobox_chip_);
	builder_->get_widget( "spinbutton-strip", spinbutton_strip_);
	builder_->get_widget( "spinbutton-code", spinbutton_code_);
	builder_->get_widget( "spinbutton-broadcast", spinbutton_broadcast_);
	builder_->get_widget( "toolbutton-load-from-file", menutoolbutton_load_from_file_);
	builder_->get_widget( "menuitem-restore-current", menuitem_restore_current_);
	builder_->get_widget( "menuitem-restore-all", menuitem_restore_all_);

	builder_->get_widget( "toolbutton-write-current", menutoolbutton_write_current_);
	builder_->get_widget( "menuitem-write-all", menuitem_write_all_);

	Glib::RefPtr<Glib::Object> obj;
	obj = builder_->get_object("liststore-strip-code");
	liststore_strip_code_ = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(obj);

	// create items
	StripCodeVector codes;
	for ( int i = 0; i < SCANNER_STRIPS_PER_CHIP_REAL; ++i) {
//		strip_code.first = i + 1; // from 1 to 128
//		strip_code.second = 0;
		codes.push_back(StripCode( i + 1, 0));
	}

	// add rows
	for ( StripCodeVector::const_iterator it = codes.begin();
		it != codes.end(); ++ it) {
		Gtk::TreeRow row = *(liststore_strip_code_->append());
		row[model_columns.strip] = it->first;
		row[model_columns.code] = it->second;
	}

	builder_->get_widget( "treeview-strip-code", treeview_strip_code_);

	selection_strip_code_ = treeview_strip_code_->get_selection();
	selection_strip_code_->set_mode(Gtk::SELECTION_SINGLE);

	// add columns
	Gtk::TreeViewColumn* column;
	Gtk::CellRenderer* renderer;
	column = new Gtk::TreeView::Column( _("Strip"), model_columns.strip);
	column->set_alignment(Gtk::ALIGN_CENTER);
	renderer = column->get_first_cell_renderer();
	renderer->property_xalign() = 1.;
	treeview_strip_code_->append_column(*Gtk::manage(column));

	column = new Gtk::TreeView::Column( _("Code"), model_columns.code);
	column->set_alignment(Gtk::ALIGN_CENTER);
	renderer = column->get_first_cell_renderer();
	renderer->property_xalign() = .5;
	treeview_strip_code_->append_column(*Gtk::manage(column));
}

void
LiningAdjustmentDialog::connect_signals()
{
	selection_strip_code_->signal_changed().connect(
		sigc::mem_fun( *this, &LiningAdjustmentDialog::on_selection_changed));

	combobox_chip_->signal_changed().connect(
		sigc::mem_fun( *this, &LiningAdjustmentDialog::on_chip_changed));

	spinbutton_strip_->signal_changed().connect(
		sigc::mem_fun( *this, &LiningAdjustmentDialog::on_strip_changed));
	spinbutton_code_->signal_changed().connect(
		sigc::mem_fun( *this, &LiningAdjustmentDialog::on_code_changed));
	spinbutton_broadcast_->signal_changed().connect(
		sigc::mem_fun( *this, &LiningAdjustmentDialog::on_broadcast_changed));

	menuitem_restore_all_->signal_activate().connect(
		sigc::mem_fun( *this, &LiningAdjustmentDialog::on_load_lining));
	menutoolbutton_write_current_->signal_clicked().connect(
		sigc::mem_fun( *this, &LiningAdjustmentDialog::on_write_lining));
	menuitem_write_all_->signal_activate().connect(
		sigc::mem_fun( *this, &LiningAdjustmentDialog::on_write_all_lining));
}

void
LiningAdjustmentDialog::update_scanner_state(const Scanner::State& state)
{
	if (!ui_state_initiated_) {
		set_lining_data(state);
		ui_state_initiated_ = true;
	}

	switch (state.manager_state().run()) {
	case RUN_BACKGROUND:
		block_interface(false);
		break;
	default:
		block_interface(true);
		break;
	}
}

void
LiningAdjustmentDialog::on_response(int)
{
	hide();
}

void
LiningAdjustmentDialog::block_interface(bool block)
{
	switch (block) {
	case true:
		break;
	case false:
		break;
	default:
		break;
	}
}

void
LiningAdjustmentDialog::set_lining_data(const Scanner::State& state)
{
	Scanner::SharedManager manager = Scanner::Manager::instance();
	Scanner::Data* data = manager->get_data();
	if (data) {

		char chip = state.chip();
		if (chip) {
			guint i = Scanner::Data::chip_number(chip);
			combobox_chip_->set_active(i);
		}

		for ( Scanner::AssemblyConstIter iter = data->begin_assemble();
			iter != data->end_assemble(); ++iter)
			lining_[iter->code] = iter->lining;

		if (chip) {
			for ( int j = 0; j < SCANNER_STRIPS_PER_CHIP_REAL; ++j)
				set_strip_code( j, lining_[chip][j]);
		}
		set_adjustment_sensitive(true);
	}
	else {
		; // Scanner data is locked by running thread
	}
}

void
LiningAdjustmentDialog::on_chip_changed()
{
	int i = combobox_chip_->get_active_row_number();
	Scanner::SharedManager manager = Scanner::Manager::instance();
	if (i != -1) {
		char chip = Scanner::Data::chip_code(i);

		Scanner::Command* com = Scanner::Commands::create(
			Scanner::COMMAND_SELECT_CHIP, chip);
		Scanner::AcquisitionParameters params(com);
		params.value = chip;

		manager->run( RUN_COMMANDS, params);

		spinbutton_strip_->set_value(0.);

		if (lining_.size()) {
			for ( int j = 0; j < SCANNER_STRIPS_PER_CHIP_REAL; ++j)
				set_strip_code( j, lining_[chip][j]);
		}
	}
}

void
LiningAdjustmentDialog::on_selection_changed()
{
	Glib::RefPtr<Gtk::TreeModel> model;
	model = Glib::RefPtr<Gtk::ListStore>::cast_static(liststore_strip_code_);

	Gtk::TreeModel::iterator iter =
		selection_strip_code_->get_selected(model);

	if (iter) {
		Gtk::TreeModel::Row row = *iter;
		int strip = row[model_columns.strip];
		int value = row[model_columns.code];
		spinbutton_strip_->set_value(strip);
		spinbutton_code_->set_value(value);
	}
}

void
LiningAdjustmentDialog::on_strip_changed()
{
	// For the TreePath strip should be from 0 to 127
	int strip = spinbutton_strip_->get_value_as_int() - 1;

	Gtk::TreePath path(Glib::ustring::format(strip));

	Gtk::TreeModel::iterator iter = liststore_strip_code_->get_iter(path);

	if (iter) {
		treeview_strip_code_->scroll_to_row(path);
		selection_strip_code_->select(iter);
	}
}

void
LiningAdjustmentDialog::on_code_changed()
{
	int i = combobox_chip_->get_active_row_number();
	if (i != -1) {
		guint strip = spinbutton_strip_->get_value_as_int() - 1;
		guint8 code = spinbutton_code_->get_value_as_int();
		char chip = Scanner::Data::chip_code(i);
		lining_[chip][strip] = code;
		set_strip_code( strip, code);
	}
}

void
LiningAdjustmentDialog::on_broadcast_changed()
{
	int i = combobox_chip_->get_active_row_number();
	if (i != -1) {
		guint8 code = spinbutton_broadcast_->get_value_as_int();
		char chip = Scanner::Data::chip_code(i);
		for ( guint strip = 0; strip < SCANNER_STRIPS_PER_CHIP_REAL; ++strip) {
			lining_[chip][strip] = code;
			set_strip_code( strip, code);
		}
	}
}

void
LiningAdjustmentDialog::set_strip_code( guint strip, guint8 code)
{
	Glib::ustring path = Glib::ustring::format(strip);
	Gtk::TreeModel::iterator iter = liststore_strip_code_->get_iter(path);
	if (iter) {
		// Store the user's new text value in the model:
		Gtk::TreeRow row = *iter;
		row[model_columns.code] = guint(code);
	}
}

LiningAdjustmentDialog*
LiningAdjustmentDialog::create()
{
	LiningAdjustmentDialog* dialog = 0;
	std::string filename(builder_lining_adjustment_dialog_filename);

	Glib::RefPtr<Gtk::Builder> builder = create_from_file(filename);

	if (builder)
		//Get the GtkBuilder-instantiated dialog
		builder->get_widget_derived( "dialog-window", dialog);

	return dialog;
}

void
LiningAdjustmentDialog::on_load_lining()
{
	Scanner::SharedManager manager = Scanner::Manager::instance();
	Scanner::Data* data = manager->get_data();
	if (data) {
		for ( Scanner::AssemblyConstIter iter = data->begin_assemble();
			iter != data->end_assemble(); ++iter)
			lining_[iter->code] = iter->lining;

		int i = combobox_chip_->get_active_row_number();
		if (i != -1) {
			char chip = Scanner::Data::chip_code(i);
			for ( int j = 0; j < SCANNER_STRIPS_PER_CHIP_REAL; ++j)
				set_strip_code( j, lining_[chip][j]);
		}
		set_adjustment_sensitive(true);
	}
	else {
		; // Scanner data is locked by running thread
	}
}

void
LiningAdjustmentDialog::on_write_lining()
{
	Scanner::SharedManager manager = Scanner::Manager::instance();
	Scanner::Data* data = manager->get_data();
	if (data) {
		int i = combobox_chip_->get_active_row_number();
		if (i != -1) {
			char chip = Scanner::Data::chip_code(i);
			data->set_chip_lining( chip, lining_[chip]);

			Scanner::Command* com =
				Scanner::Commands::create( chip, lining_[chip]);
			Scanner::AcquisitionParameters params(com);
			manager->run( RUN_COMMANDS, params);
		}
	}
}

void
LiningAdjustmentDialog::on_write_all_lining()
{
	Scanner::SharedManager manager = Scanner::Manager::instance();
	Scanner::Data* data = manager->get_data();
	if (data) {
		data->set_lining(lining_);
	}

	std::vector<Scanner::Command*> coms;

	for ( LiningConstIter it = lining_.begin(); it != lining_.end(); ++it) {
		Scanner::Command* com = Scanner::Commands::create( it->first, it->second);
		coms.push_back(com);
	}

	Scanner::AcquisitionParameters params(coms);
	manager->run( RUN_COMMANDS, params);
}

void
LiningAdjustmentDialog::set_adjustment_sensitive(bool state)
{
	spinbutton_broadcast_->set_sensitive(state);
	spinbutton_strip_->set_sensitive(state);
	spinbutton_code_->set_sensitive(state);
}

} // namespace UI

} // namespace ScanAmati
