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

#include <tr1/memory>

#include <gtkmm/window.h>
#include <gtkmm/actiongroup.h>
#include <gtkmm/pagesetup.h>
#include <gtkmm/printsettings.h>
#include <gtkmm/builder.h>
#include <gtkmm/uimanager.h>
#include <gtkmm/radioaction.h>

#include "widgets/files_icon_view.hpp"

#include "scanner/data.hpp"

#include "print/operation.hpp"

namespace Gtk {
class Dialog;
class FileFilter;
class MenuBar;
class Toolbar;
class HPaned;
class VPaned;
class VBox;
};

namespace ScanAmati {

namespace Scanner {
class State;
} // namespace Scanner

namespace DICOM {
class SummaryInfo;
} // namespace DICOM

namespace UI {

struct ActionState;

// Widgets
class PaletteArea;
class ImageArea;
class ScrolledImageArea;
class InformationNotebook;
class Statusbar;

// Dialogs
class ScannerDebugDialog;
class TemperatureMarginsDialog;
class ImageAcquisitionDialog;
class LiningAcquisitionDialog;
class ConquestArchiveDialog;

class MainWindow : public Gtk::Window {

public:
	static MainWindow* create(const Glib::OptionGroup::vecustrings&);
	MainWindow( BaseObjectType* cobject,
		const Glib::RefPtr<Gtk::Builder>& builder);
	virtual ~MainWindow();
	void close();

protected:
	// vfunc overrides:
	virtual bool on_delete_event(GdkEventAny*);

	// Member functions:
	void connect_signals();
	void init_actions();
	void init_ui();
	void init_ui_manager();
	void init_actions_state(bool state);

	void show_menus_bars();
	void show_image_area();
	void show_info_notebook();
	void show_files_view();
	void show_top_paned();
	void show_statusbar();

	// signal handlers:
	void on_init();
	void on_quit();
	void on_file_open();
	void on_file_save();
	void on_file_save_as();
	void on_file_save_archive();
	void on_preferences();
	void on_temperature_margins();
	void on_image_find();
	void on_image_acquisition();
	void on_image_ready();
	void on_printoperation_status_changed(
		const Glib::RefPtr<Print::Operation>&);
	void on_printoperation_done( Gtk::PrintOperationResult result,
		const Glib::RefPtr<Print::Operation>&);
	void on_print_preview(Gtk::PrintOperationAction);
	void on_print_page_setup();
	void on_image_reconstruction(const boost::any&);
	void on_lining_acquisition();
	void on_write_lining();
	void on_scanner_device(bool);
	void on_scanner_debug();
	void on_new_dicom_info();
	void on_edit_dicom_info();
	void on_dicom_info_changed(const DICOM::SummaryInfo& info);
	void on_acquisition_flag_changed(bool state) { acquire_data_ = state; }
	void on_file_loaded(bool) {}
	void on_file_saved(bool) {}
	void on_file_saved_archive(bool) {}
	void on_file_view_state_type(FilesIconView::StateType);
	void on_file_view_data_type();
	void on_images_cleaned();
	void load_file( const std::string&, bool);
	void update_scanner_state();
	void update_data_state();
	void set_actions_state(const ActionState* acts);
	void set_actions_state( const ActionState* acts, bool state);

	// UI widgets and members:
	Glib::RefPtr<Gtk::Builder> builder_;
	Glib::RefPtr<Gtk::UIManager> ui_manager_; // menus and toolbars items
	Glib::RefPtr<Gtk::ActionGroup> action_group_; // menus and toolbars actions
	Gtk::HPaned* hpaned_top_; // hpaned container
	Gtk::VPaned* vpaned_top_; // vpaned container
	ScrolledImageArea* scrolled_image_area_;
	PaletteArea* palette_area_; // pointer for a PaletteArea widget
	ImageArea* image_area_; // pointer for a ImageArea widget
	InformationNotebook* info_notebook_;
	FilesIconView* files_view_; // widget stores icons and files data
	Statusbar* statusbar_;

	bool acquire_data_; // data acquisition flag
	Glib::OptionGroup::vecustrings files_;

	// dialogs
	std::tr1::shared_ptr<TemperatureMarginsDialog> temperature_margins_dialog_;
	std::tr1::shared_ptr<ImageAcquisitionDialog> image_acquisition_dialog_;
	std::tr1::shared_ptr<LiningAcquisitionDialog> lining_acquisition_dialog_;
	std::tr1::shared_ptr<ScannerDebugDialog> scanner_debug_dialog_;
	std::tr1::shared_ptr<ConquestArchiveDialog> conquest_archive_dialog_;

	// Printing-related objects:
	Glib::RefPtr<Gtk::PageSetup> page_setup_;
	Glib::RefPtr<Gtk::PrintSettings> print_settings_;

	// signals
	sigc::signal< void, const Scanner::State&> signal_scanner_state_changed_;
	sigc::signal<void> signal_scanner_data_ready_;
	sigc::signal<void> signal_scanner_image_ready_;
	sigc::signal< void, bool> signal_device_connection_;
};

} // namespace UI

} // namespace ScanAmati
