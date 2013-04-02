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

#include <gtkmm/liststore.h>
#include <gtkmm/builder.h>
#include <gtkmm/iconview.h>

// files from src directory begin
#include "file.hpp"
// files from src directory end

namespace Gtk {
class Menu;
}

namespace ScanAmati {

namespace Image {
class Palette;
}

namespace UI {

class FilesIconView : public Gtk::IconView {

public:
	enum StateType {
		STATE_NEW_DATA,
		STATE_FILE_DATA,
		STATE_PACS_DATA,
		STATE_FILE_SAVED,
		STATE_PACS_SAVED,
		STATE_CHANGED
	};

	FilesIconView( BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>&);
	virtual ~FilesIconView();

	void add_file( const std::string& filename, File& file,
		bool from_pacs = false);
	void add_image_data(const Image::SummaryData&);

	void change_current_filename(const std::string& file);
	void change_current_image_data(const Image::SummaryData&);
	void change_current_dicom_info(const DICOM::SummaryInfo&);
	void set_new_dicom_info(const DICOM::SummaryInfo&);
	void remove_current();
	void clear_all();
	bool get_current_filename(std::string& filename);
	bool get_current_data( Image::SummaryData& data,
		DICOM::SummaryInfo& dicom_summary);
	bool get_current_data(DICOM::SummaryInfo& dicom_summary);
	bool get_current_data(DICOM::Dataset&);
	void set_popup_menus( Gtk::Menu*, Gtk::Menu*);
	void set_palette(const Image::Palette*);

	sigc::signal< void, const Image::SummaryData&> signal_image_data_clicked();
	sigc::signal< void, const DICOM::SummaryInfo&> signal_dicom_info_clicked();
	sigc::signal< void, StateType> signal_state_type_clicked();
	sigc::signal<void> signal_images_cleaned();

protected:
	virtual bool on_button_press_event(GdkEventButton*);
	virtual bool on_motion_notify_event(GdkEventMotion*);
	virtual void on_item_activated(const Gtk::TreeModel::Path&);
	virtual void on_selection_changed();
	Glib::RefPtr<Gdk::Pixbuf> create_icon_pixbuf(
		const Image::DataSharedPtr&,
		const std::vector<guint8>&,
		unsigned int size = 64);

	// Methods:
	void init_ui();
	void connect_signals();

	// Members:
	Glib::RefPtr<Gtk::Builder> builder_;
	Glib::RefPtr<Gtk::ListStore> liststore_icons_;
	Gtk::Menu* menu_view_;
	Gtk::Menu* menu_icons_;
	Gtk::TreeModel::Path current_path_;
	Gtk::TreeModel::Path pointer_path_;
	bool item_selected_;

	const struct ModelColumns : public Gtk::TreeModel::ColumnRecord {
		ModelColumns() {
			add(icon);
			add(label);
			add(tooltip_label);
			add(filename);
			add(dicom_info);
			add(image_data);
			add(state);
		}

		Gtk::TreeModelColumn< Glib::RefPtr<Gdk::Pixbuf> > icon;
		Gtk::TreeModelColumn<Glib::ustring> label;
		Gtk::TreeModelColumn<Glib::ustring> tooltip_label;
		Gtk::TreeModelColumn<std::string> filename;
		Gtk::TreeModelColumn<DICOM::SummaryInfo> dicom_info;
		Gtk::TreeModelColumn<Image::SummaryData> image_data;
		Gtk::TreeModelColumn<StateType> state;
	} model_columns;

	DICOM::SummaryInfo new_dicom_info_;

	// Signals:
	sigc::signal< void, const Image::SummaryData&> signal_image_data_clicked_;
	sigc::signal< void, const DICOM::SummaryInfo&> signal_dicom_info_clicked_;
	sigc::signal< void, StateType> signal_state_type_clicked_;
	sigc::signal<void> signal_images_cleaned_;
};

inline
void
FilesIconView::set_popup_menus( Gtk::Menu* menu_view, Gtk::Menu* menu_icons)
{
	menu_view_ = menu_view;
	menu_icons_ = menu_icons;
}

} // namespace UI

} // namespace ScanAmati
