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

#include <gtkmm/notebook.h>
#include <gtkmm/builder.h>
#include <gtkmm/treestore.h>

namespace Gtk {
class TreeView;
}

namespace ScanAmati {

namespace DICOM {
class SummaryInfo;
} // namespace DICOM

class XrayParameters;

namespace Scanner {
class State;
} // namespace Scanner

namespace UI {

struct InfoItem;

class InformationNotebook : public Gtk::Notebook {

public:
	InformationNotebook( BaseObjectType* cobject,
		const Glib::RefPtr<Gtk::Builder>&);
	virtual ~InformationNotebook();
	virtual void update_scanner_state(const Scanner::State&);
	void clear_dicom_info();
	void on_xray_connection(bool state);
	void on_xray_parameters(const XrayParameters&);
	void on_dicom_info(const DICOM::SummaryInfo&);
	void on_xray_info(const DICOM::SummaryInfo&);
	void on_array_info(const DICOM::SummaryInfo&);

protected:
	// vfunc overrides:
	virtual void on_realize();

	// Member functions:
	void create_model();

	// ui:
	void init_ui();
	void connect_signals();

	struct ModelColumns;
	void add_items();
	void add_treeview_columns( Gtk::TreeView*, const ModelColumns&);
	void add_treemodel_item( Glib::RefPtr<Gtk::TreeStore>& treemodel,
		const ModelColumns& columns, const InfoItem& foo);

	// Member widgets:
	Glib::RefPtr<Gtk::Builder> builder_;
	Gtk::TreeView* treeview_study_;
	Gtk::TreeView* treeview_scanner_;
	Glib::RefPtr<Gtk::TreeStore> treestore_study_;
	Glib::RefPtr<Gtk::TreeStore> treestore_scanner_;

	const struct ModelColumns : public Gtk::TreeModelColumnRecord {
		Gtk::TreeModelColumn< Glib::RefPtr<Gdk::Pixbuf> > icon;
		Gtk::TreeModelColumn<Glib::ustring> label;
		Gtk::TreeModelColumn<Glib::ustring> value;
		Gtk::TreeModelColumn<Glib::ustring> color;
		Gtk::TreeModelColumn<bool> visible;
		ModelColumns() {
			add(icon);
			add(label);
			add(value);
			add(color);
			add(visible);
		}
	} columns_study_, columns_scanner_;

	std::vector<InfoItem> items_study_, items_scanner_;
private:
	void set_study_path_value( const char* path,
		const Glib::ustring& value,
		const Glib::ustring& color = "black");
	void set_scanner_path_value( const char* path,
		const Glib::ustring& value,
		const Glib::ustring& color = "black");
	void set_scanner_path_value( const char* path,
		const Glib::RefPtr<Gdk::Pixbuf>& icon,
		const Glib::ustring& label,
		const Glib::ustring& value,
		const Glib::ustring& color = "black");
};

} // namespace UI

} // namespace ScanAmati
