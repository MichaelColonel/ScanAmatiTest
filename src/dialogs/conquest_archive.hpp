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

#include <gtkmm/dialog.h>
#include <gtkmm/treestore.h>
#include <gtkmm/builder.h>

#include <pqxx/connection>

namespace Gtk {
class TreeView;
class Entry;
class CheckButton;
class Label;
class Button;
class TreeViewColumn;
} // namespace Gtk

class DcmDataset;

namespace ScanAmati {

namespace UI {

struct ArchiveItem;
class MoveDicomDialog;

class ConquestArchiveDialog : public Gtk::Dialog {
public:
	ConquestArchiveDialog( BaseObjectType* cobject,
		const Glib::RefPtr<Gtk::Builder>& builder);
	virtual ~ConquestArchiveDialog();
	sigc::signal< void, const std::string&> signal_moved_filename();
	static ConquestArchiveDialog* create();

protected:
	void init_ui();
	void connect_signals();
	void create_columns();
	void connect_database(const char* conninfo);
	void load_preferences();
	void save_preferences();
	void fill_dicom_query( Gtk::TreeModel::Row&, DcmDataset&);

	void add_archive_item(const ArchiveItem&);

	virtual void on_response(int);
	void on_move_result( bool, const Glib::ustring&, MoveDicomDialog*);
	void on_find_clicked();
	void on_clear_clicked();
	void on_treeview_row_activated( const Gtk::TreeModel::Path&,
		Gtk::TreeViewColumn*);

	// GUI members
	Glib::RefPtr<Gtk::Builder> builder_;
	Glib::RefPtr<Gtk::TreeStore> treestore_;

	Gtk::TreeView* treeview_;

	// find GUI members
	Gtk::Entry* entry_patient_name_;
	Gtk::Entry* entry_patient_id_;
	Gtk::Entry* entry_study_id_;
	Gtk::CheckButton* checkbutton_case_sensitive_;
	Gtk::Label* label_found_;
	Gtk::Button* button_additional_query_;
	Gtk::Button* button_find_;
	Gtk::Button* button_clear_;

	std::vector<ArchiveItem> archive_items_;
	pqxx::connection* conn_;
	sigc::signal< void, const std::string&> signal_moved_filename_;
};

} // namespace UI

} // namespace ScanAmati
