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
#include <gtkmm/frame.h>
#include <gtkmm/filefilter.h>
#include <gtkmm/paned.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/builder.h>

#include <gtkmm/menubar.h>
#include <gtkmm/menu.h>
#include <gtkmm/toolbar.h>

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include <glibmm/i18n.h>

#include <boost/any.hpp>

#include "widgets/scrolled_image_area.hpp"
#include "widgets/image_area.hpp"
#include "widgets/palette_area.hpp"
#include "widgets/information_notebook.hpp"
#include "widgets/status_bar.hpp"

#include "dialogs/image_acquisition.hpp"
#include "dialogs/temperature_margins.hpp"
#include "dialogs/lining_acquisition.hpp"
#include "dialogs/scanner_debug.hpp"
#include "dialogs/store_dicom.hpp"
#include "dialogs/conquest_archive.hpp"
#include "dialogs/dicom_information.hpp"
#include "dialogs/preferences.hpp"
#include "dialogs/save_as.hpp"
#include "dialogs/utils.hpp"

#include "dicom/utils.hpp"

#include "scanner/manager.hpp"

#include "file_loader.hpp"
#include "file_saver.hpp"
#include "application.hpp"
#include "global_strings.hpp"
#include "preferences.hpp"
#include "utils.hpp"

#include "icon_loader.hpp"

#include "main_window.hpp"

namespace ScanAmati {

namespace UI {

const ActionState init_actions_basic[] = {
	{ "action-device-connect", false },
	{ "action-device-disconnect", true },
	{ "action-lining-acquisition", true },
	{ "action-image-acquisition", true },
	{ "action-temperature-margins", true },
	{ } // // Terminating Entry
};

const ActionState init_actions_extend[] = { 
	{ "action-device-connect", false },
	{ "action-device-disconnect", true },
	{ "action-lining-acquisition", true },
	{ "action-image-acquisition", true },
	{ "action-temperature-margins", true },
	{ "action-scanner-debug", true },
	{ } // Terminating Entry
};

MainWindow::MainWindow( BaseObjectType* cobject,
	const Glib::RefPtr<Gtk::Builder>& builder)
	:
	Gtk::Window(cobject),
	builder_(builder),
	hpaned_top_(0),
	vpaned_top_(0),
	scrolled_image_area_(0),
	palette_area_(0),
	image_area_(0),
	info_notebook_(0),
	files_view_(0),
	statusbar_(0),
	acquire_data_(true)
{
	init_ui();

	connect_signals();

	Glib::RefPtr<Gdk::Pixbuf> icon = IconLoader::load_program_icon(48);

	set_icon(icon);

	int h = app.prefs.get<int>( "Gui", conf_key_window_height);
	int w = app.prefs.get<int>( "Gui", conf_key_window_width);
	int x = app.prefs.get<int>( "Gui", conf_key_window_position_x);
	int y = app.prefs.get<int>( "Gui", conf_key_window_position_y);

	resize( w, h);
	move( x, y);
			
	show_all_children();
}

MainWindow::~MainWindow()
{
}

/** \brief Main user interface loading function.
 *
 * Calls all the small functions in correct order.
 */
void
MainWindow::init_ui()
{
	show_top_paned();

	show_info_notebook();

	show_image_area();

	show_files_view();

	show_statusbar();

	init_ui_manager();

	init_actions();

	show_menus_bars();
}

void
MainWindow::show_top_paned()
{
	builder_->get_widget( "hpaned-top", hpaned_top_);
	builder_->get_widget( "vpaned-top", vpaned_top_);
}

void
MainWindow::show_statusbar()
{
	builder_->get_widget_derived( "statusbar", statusbar_);
}

void
MainWindow::show_info_notebook()
{
	builder_->get_widget_derived( "notebook-info", info_notebook_);
}

void
MainWindow::show_files_view()
{
	builder_->get_widget_derived( "iconview-files", files_view_);

	int pos = app.prefs.get<int>( "Gui", conf_key_window_hpaned_position);
	hpaned_top_->set_position(pos);
}

void
MainWindow::show_image_area()
{
	builder_->get_widget_derived( "table-scrolled-widget", scrolled_image_area_);
	image_area_ = scrolled_image_area_->image_area_widget();
	palette_area_ = scrolled_image_area_->palette_area_widget();

	int pos = app.prefs.get<int>( "Gui", conf_key_window_vpaned_position);
	vpaned_top_->set_position(pos);
}

/**
 * Initializes UIManager with the action group and UI description file.
 */
void
MainWindow::init_ui_manager()
{
	ui_manager_ = Gtk::UIManager::create();
	Gtk::UIManager::ui_merge_id id =
		ui_manager_->add_ui_from_file(uimanager_menus_bars_filename);

	if (!id) {
		// We can't do anything without the UI definition.
		OFLOG_FATAL( app.log, "Building menus and toolbars failed.");
		exit(EXIT_FAILURE);
	}
}

/**
 * Loads menu and toolbar objects from UIManager, packs them in the VBox.
 */
void
MainWindow::show_menus_bars()
{
	// Get top container
	Gtk::VBox* vbox;
	builder_->get_widget( "vbox-top", vbox);

	// Get the menu and toolbar objects from UIManager.
	Gtk::MenuBar* menubar =
		dynamic_cast<Gtk::MenuBar*>(ui_manager_->get_widget("/menubar"));
	g_assert(menubar);

	Gtk::Toolbar* toolbar =
		dynamic_cast<Gtk::Toolbar*>(ui_manager_->get_widget("/toolbar"));
	g_assert(toolbar);

	// Pack them on their VBox.
	vbox->pack_start( *menubar, Gtk::PACK_SHRINK);
	vbox->reorder_child( *menubar, 0);

	toolbar->set_toolbar_style(Gtk::TOOLBAR_BOTH);
	vbox->pack_start( *toolbar, Gtk::PACK_SHRINK);
	vbox->reorder_child( *toolbar, 1);

	// Popup menus must be added to respective widgets.

	// Get menu objects from UIManager and add them to the proper widgets.
	Gtk::Widget* widget = ui_manager_->get_widget("/popup-menu-palette");
	Gtk::Menu* menu = dynamic_cast<Gtk::Menu*>(widget);
	g_assert(menu);
	palette_area_->set_popup_menu(menu);

	widget = ui_manager_->get_widget("/popup-image-area");
	menu = dynamic_cast<Gtk::Menu*>(widget);
	g_assert(menu);
	image_area_->set_popup_menu(menu);

	widget = ui_manager_->get_widget("/popup-files-view");
	menu = dynamic_cast<Gtk::Menu*>(widget);
	g_assert(menu);

	widget = ui_manager_->get_widget("/popup-file-icons");
	Gtk::Menu* menu_icons = dynamic_cast<Gtk::Menu*>(widget);
	files_view_->set_popup_menus( menu, menu_icons);
}

/**
 * Connect widgets and members signals
 */
void
MainWindow::connect_signals()
{
	// Main Window:
	signal_device_connection_.connect(sigc::mem_fun(
		*this, &MainWindow::init_actions_state));
	signal_scanner_data_ready_.connect( sigc::bind( sigc::mem_fun(
		*this, &MainWindow::on_image_reconstruction), boost::any()));
	signal_scanner_image_ready_.connect(sigc::mem_fun(
		*this, &MainWindow::on_image_ready));

	// Widgets:
	signal_scanner_state_changed_.connect(sigc::mem_fun(
		*info_notebook_, &InformationNotebook::update_scanner_state));
	signal_scanner_state_changed_.connect(sigc::mem_fun(
		*statusbar_, &Statusbar::update_scanner_state));

	files_view_->signal_image_data_clicked().connect(sigc::mem_fun(
		*image_area_, &ImageArea::set_image_data));
	files_view_->signal_images_cleaned().connect(sigc::mem_fun(
		*image_area_, &ImageArea::clear_area));
	files_view_->signal_dicom_info_clicked().connect(sigc::mem_fun(
		*info_notebook_, &InformationNotebook::on_dicom_info));
	files_view_->signal_images_cleaned().connect(sigc::mem_fun(
		*info_notebook_, &InformationNotebook::clear_dicom_info));
	files_view_->signal_dicom_info_clicked().connect(sigc::mem_fun(
		*this, &MainWindow::on_dicom_info_changed));
	files_view_->signal_state_type_clicked().connect(sigc::mem_fun(
		*this, &MainWindow::on_file_view_state_type));
	files_view_->signal_data_type_clicked().connect(sigc::mem_fun(
		*this, &MainWindow::on_file_view_data_type));
	files_view_->signal_images_cleaned().connect(sigc::mem_fun(
		*this, &MainWindow::on_images_cleaned));

	palette_area_->signal_palette_changed().connect(
		sigc::mem_fun( *image_area_, &ImageArea::set_palette));
	palette_area_->signal_palette_changed().connect(
		sigc::mem_fun( *files_view_, &FilesIconView::set_palette));

	// Scanner manager
	Scanner::SharedManager manager = Scanner::Manager::instance();

	manager->signal_update().connect(
		sigc::mem_fun( *this, &MainWindow::update_scanner_state));

	Scanner::Data* data = manager->get_data();
	if (data) {
		data->signal_complete().connect(
			sigc::mem_fun( *this, &MainWindow::update_data_state));
	}

	// Initiation
	Glib::signal_idle().connect(sigc::bind_return(
		sigc::mem_fun( *this, &MainWindow::on_init), false));
}

void
MainWindow::on_file_open()
{
	Gtk::FileChooserDialog dialog( _("Open File"),
		Gtk::FILE_CHOOSER_ACTION_OPEN);

	dialog.set_transient_for(*this);

	// Add response buttons in the dialog:
	dialog.add_button( Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	dialog.add_button( Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

	// Add filters, so that only certain file types can be selected:
	Gtk::FileFilter filter_dcm;
	filter_dcm.set_name(gettext(filter_dcm_name));
	filter_dcm.add_mime_type("application/dicom");
	dialog.add_filter(filter_dcm);

	Gtk::FileFilter filter_raw;
	if (app.extend) {
		filter_raw.set_name(gettext(filter_raw_name));
		filter_raw.add_pattern("*.raw");
		filter_raw.add_pattern("*.RAW");
		dialog.add_filter(filter_raw);
	}

	// Show the dialog and wait for a user response:
	int result = dialog.run();

	// Handle the response:
	switch (result) {
    case Gtk::RESPONSE_OK:
		{
			// Notice that this is a std::string, not a Glib::ustring.
			std::string filename = dialog.get_filename();

			const Gtk::FileFilter* filter = dialog.get_filter();
			try {
				FileLoader loader( filename, filter);
				loader.signal_file_loaded().connect(sigc::bind( sigc::mem_fun(
					*files_view_, &FilesIconView::add_file), false));
				loader.load();
			}
			catch (const Exception& ex) {
				std::cerr << ex.what() << std::endl;
			}
		}
		break;
    case Gtk::RESPONSE_CANCEL:
		break;
	default:
		break;
	}
}

void
MainWindow::on_file_save()
{
}

void
MainWindow::on_file_save_as()
{
	std::string filename;
	bool res = files_view_->get_current_filename(filename);
	if (!res)
		return;

	SaveAsDialog dialog(*this);

	if (!filename.empty()) {
		dialog.set_current_name(Glib::path_get_basename(filename));

		std::list< std::string > parts = Glib::Regex::split_simple(
			dot_pattern, filename);
		if (parts.size() >= 1) {
			filename = *parts.begin();
		}
		dialog.set_filename(filename);
	}

	// Show the dialog and wait for a user response:
	int result = dialog.run();

	// Handle the response:
	switch (result) {
    case Gtk::RESPONSE_OK:
    {
		Image::SummaryData image_data;
		DICOM::SummaryInfo dicom_info;
		files_view_->get_current_data( image_data, dicom_info);

		try {
			const Gtk::FileFilter* filter = dialog.get_filter();

			filename = dialog.get_filename();

			FileSaver saver( filename, filter);
			saver.signal_file_saved().connect(sigc::mem_fun(
				*files_view_, &FilesIconView::change_current_filename));

			Glib::RefPtr<Gdk::Pixbuf> pixbuf = image_area_->get_pixbuf();
			res = saver.save( image_data.presentation_data(), pixbuf, dicom_info);
		}
		catch (const Exception& ex) {
			ErrorDialog dialog_( dialog, _("Saving Error"));

			Glib::ustring msg = Glib::ustring::compose(
				_("Unable to save the file \"%1\"."),
				Glib::filename_to_utf8(Glib::path_get_basename(filename)));

			dialog_.set_message(msg);
			dialog_.set_secondary_text(ex.what());
			dialog_.run();
		}
	}
		break;
    case Gtk::RESPONSE_CANCEL:
		break;
	default:
		break;
	}
}

void
MainWindow::on_file_save_archive()
{
	DICOM::Dataset dataset;
	if (files_view_->get_current_data(dataset)) {
		StoreDicomDialog *dialog = StoreDicomDialog::create(&dataset);
		int res = dialog->run();
		delete dialog;
	}
}

void
MainWindow::on_image_find()
{
	if (!conquest_archive_dialog_) {
		ConquestArchiveDialog* dialog = ConquestArchiveDialog::create();
		if (dialog) {
			dialog->signal_moved_filename().connect(sigc::bind( sigc::mem_fun(
				*this, &MainWindow::load_file), true));

			conquest_archive_dialog_.reset(dialog);
		}
	}
	if (conquest_archive_dialog_)
		conquest_archive_dialog_->run();
}

void
MainWindow::on_quit()
{
	int w = 0;
	int h = 0;
	int x = 0;
	int y = 0;
	int hpos, vpos = 0;

	get_size( w, h);
	get_position( x, y);
	hpos = hpaned_top_->get_position();
	vpos = vpaned_top_->get_position();

	app.prefs.set( "Gui", conf_key_window_height, h);
	app.prefs.set( "Gui", conf_key_window_width, w);
	app.prefs.set( "Gui", conf_key_window_position_x, x);
	app.prefs.set( "Gui", conf_key_window_position_y, y);
	app.prefs.set( "Gui", conf_key_window_hpaned_position, hpos);
	app.prefs.set( "Gui", conf_key_window_vpaned_position, vpos);

	Scanner::SharedManager manager = Scanner::Manager::instance();
	manager->stop(true);

	hide();
}

bool
MainWindow::on_delete_event(GdkEventAny*)
{
	on_quit();
	return false;
}

void
MainWindow::on_init()
{
	init_actions_state(false);

	for ( Glib::OptionGroup::vecustrings::const_iterator it = files_.begin();
		it != files_.end(); ++it) {
		load_file( it->raw(), false);
	}
}

void
MainWindow::close()
{
	on_quit();
}

void
MainWindow::on_preferences()
{
	PreferencesDialog* dialog = PreferencesDialog::create();
	if (dialog) {
		dialog->run();
		delete dialog;
	}
}

void
MainWindow::on_image_ready()
{
	Scanner::SharedManager manager = Scanner::Manager::instance();
	OFLOG_DEBUG( app.log, "Image has beed ready");

	if (Scanner::Data* data = manager->get_data()) {
		Image::SummaryData image = data->get_summary_data();
		files_view_->add_image_data(image);
	}

}

void
MainWindow::set_actions_state( const ActionState* acts, bool state)
{
	int i = 0;
	Glib::RefPtr<Gtk::Action> act;
	while (acts[i].action_name) {
		act = action_group_->get_action(acts[i].action_name);
		bool active = (state) ? acts[i].sensitivity : !acts[i].sensitivity;
		act->set_sensitive(active);
		++i;
	}
}

void
MainWindow::set_actions_state(const ActionState* acts)
{
	int i = 0;
	Glib::RefPtr<Gtk::Action> act;
	while (acts[i].action_name) {
		act = action_group_->get_action(acts[i].action_name);
		act->set_sensitive(acts[i].sensitivity);
		++i;
	}
}

void
MainWindow::init_actions_state(bool state)
{
	const ActionState* actions = app.extend ? init_actions_extend
		: init_actions_basic;

	set_actions_state( actions, state);
}

/**
 * Connect the scanner if connect is true disconnect otherwise
 */
void
MainWindow::on_scanner_device(bool connect)
{
	Scanner::SharedManager manager = Scanner::Manager::instance();
	if (connect) {
		manager->run(RUN_INITIATION);
	}
	else
		manager->stop(true);
}

void
MainWindow::on_temperature_margins()
{
	if (!temperature_margins_dialog_) {
		TemperatureMarginsDialog* dialog = TemperatureMarginsDialog::create();
		if (dialog) {
			// Connect signals:
			signal_scanner_state_changed_.connect(sigc::mem_fun(
				*dialog, &TemperatureMarginsDialog::update_scanner_state));
			temperature_margins_dialog_.reset(dialog);
		}
	}
	if (temperature_margins_dialog_) {
		temperature_margins_dialog_->run();
	}
}

void
MainWindow::on_image_acquisition()
{
	if (!image_acquisition_dialog_) {
		ImageAcquisitionDialog* dialog = ImageAcquisitionDialog::create();
		if (dialog) {
			// Connect signals:
			signal_scanner_state_changed_.connect(sigc::mem_fun(
				*dialog, &ImageAcquisitionDialog::update_scanner_state));
			image_acquisition_dialog_.reset(dialog);
		}
	}
	int result = image_acquisition_dialog_->run();

	switch (result) {
	case 0:
	default:
		break;
	}
}

void
MainWindow::on_lining_acquisition()
{
	if (!lining_acquisition_dialog_) {
		LiningAcquisitionDialog* dialog = LiningAcquisitionDialog::create();
		if (dialog) {
			// Connect signals:
			signal_scanner_state_changed_.connect(sigc::mem_fun(
				*dialog, &LiningAcquisitionDialog::update_scanner_state));
			lining_acquisition_dialog_.reset(dialog);
		}
	}
	if (lining_acquisition_dialog_)
		lining_acquisition_dialog_->run();
}

void
MainWindow::on_image_reconstruction(const boost::any& value)
{
	Scanner::SharedManager manager = Scanner::Manager::instance();
	manager->run(RUN_IMAGE_RECONSTRUCTION);

	Scanner::Data* data = manager->get_data();
	if (data)
		data->run_image_reconstruction(value);
}

void
MainWindow::on_scanner_debug()
{
	if (!scanner_debug_dialog_) {
		ScannerDebugDialog* dialog = ScannerDebugDialog::create();
		if (dialog) {
			// Connect signals:
			signal_scanner_state_changed_.connect(sigc::mem_fun(
				*dialog, &ScannerDebugDialog::update_scanner_state));
			scanner_debug_dialog_.reset(dialog);
		}
	}
	if (scanner_debug_dialog_)
		scanner_debug_dialog_->run();
}

void
MainWindow::load_file( const std::string& filename, bool from_pacs)
{
	try {
		FileLoader loader(filename);
		loader.signal_file_loaded().connect(sigc::bind( sigc::mem_fun(
			*files_view_, &FilesIconView::add_file), from_pacs));
		loader.load();
	}
	catch (const Exception& ex) {
	}
}

void
MainWindow::update_scanner_state()
{
	Scanner::SharedManager manager = Scanner::Manager::instance();

	Scanner::State scanner_state = manager->get_state();
	Scanner::ManagerState state = scanner_state.manager_state();

	if (state.process_error() && state.device_disconnected()) {
		Glib::ustring what, todo;
		scanner_state.what_todo( what, todo);

		ErrorDialog dialog( *this, _("Scanner Error"));
		dialog.set_message(what);
		dialog.set_secondary_text(todo);
		dialog.run();
	}

	switch (state.run()) {
	case RUN_INITIATION:
		if (state.process_finished()) {
			manager->join_run_thread();
			signal_device_connection_(true);
		}
		break;
	case RUN_NONE:
		switch (state.process()) {
		case PROCESS_NONE:
			manager->join_run_thread();
			signal_device_connection_(false);
			break;
		default:
			break;
		}
	case RUN_COMMANDS:
		if (state.process_finished())
			manager->join_run_thread();
		break;
	case RUN_IMAGE_ACQUISITION:
		switch (state.process()) {
		case PROCESS_START:
			break;
		case PROCESS_FINISH:
			manager->join_run_thread();
			if (acquire_data_)
				signal_scanner_data_ready_(); // start image reconstruction
			break;
		default:
			break;
		}
		break;
	case RUN_LINING_ACQUISITION:
		if (state.process_finished())
			manager->join_run_thread();
		break;
	case RUN_IMAGE_RECONSTRUCTION:
	case RUN_BACKGROUND:
	default:
		break;
	}

	signal_scanner_state_changed_(scanner_state);
}

void
MainWindow::update_data_state()
{
	Scanner::SharedManager manager = Scanner::Manager::instance();
	manager->join_data_thread();
	manager->stop(false);

	signal_scanner_image_ready_(); // send signal that image is ready
}

void
MainWindow::on_dicom_info_changed(const DICOM::SummaryInfo& info)
{
	DICOM::SummaryInfo* summary = const_cast<DICOM::SummaryInfo*>(&info);
	DICOM::PatientInfo* patient = summary->get_patient_info();
	DICOM::StudyInfo* study = summary->get_study_info();

	Glib::ustring title = Glib::ustring::compose(
		"%1 \u2014 %2 (%3) : %4", Glib::ustring( _("ScanAmati")),
		DICOM::format_person_name(patient->get_name()),
		summary->get_patient_age().age_string(), study->get_id());

	set_title(title);
}

void
MainWindow::on_new_dicom_info()
{
	DicomInformationDialog* dialog = 0;
	dialog = DicomInformationDialog::create();
	if (dialog) {
		dialog->signal_new_dicom_info().connect(sigc::mem_fun(
			*files_view_, &FilesIconView::set_new_dicom_info));
		dialog->run();
		delete dialog;
	}
}

void
MainWindow::on_edit_dicom_info()
{
	DicomInformationDialog* dialog = 0;
	dialog = DicomInformationDialog::create();

	if (dialog) {
		DICOM::SummaryInfo info;
		if (files_view_->get_current_data(info))
			dialog->set_dicom_info(info);

		dialog->signal_new_dicom_info().connect(sigc::mem_fun(
			*files_view_, &FilesIconView::change_current_dicom_info));
		dialog->run();
		delete dialog;
	}
}

void
MainWindow::on_printoperation_status_changed(
	const Glib::RefPtr<Print::Operation>& operation)
{
	Glib::ustring msg = (operation->is_finished()) ? "Print job completed." :
		operation->get_status_string();
}

void
MainWindow::on_printoperation_done( Gtk::PrintOperationResult result,
	const Glib::RefPtr<Print::Operation>& operation)
{
	//Printing is "done" when the print data is spooled.
	if (result == Gtk::PRINT_OPERATION_RESULT_ERROR) {
		Gtk::MessageDialog dialog( *this, "Error printing form", false,
			Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
		dialog.run();
	}
	else if (result == Gtk::PRINT_OPERATION_RESULT_APPLY) {
		//Update PrintSettings with the ones used in this PrintOperation:
		print_settings_ = operation->get_print_settings();
	}

	if (!operation->is_finished()) {
		//We will connect to the status-changed signal to track status
		//and update a status bar. In addition, you can, for example,
		//keep a list of active print operations, or provide a progress dialog.
		operation->signal_status_changed().connect(sigc::bind( sigc::mem_fun(
			*this, &MainWindow::on_printoperation_status_changed), operation));
	}
}

void
MainWindow::on_print_page_setup()
{
	//Show the page setup dialog, asking it to start with the existing settings:
	Glib::RefPtr<Gtk::PageSetup> new_page_setup =
		Gtk::run_page_setup_dialog(*this, page_setup_, print_settings_);

	page_setup_ = new_page_setup;
}

void
MainWindow::on_print_preview(Gtk::PrintOperationAction action)
{
	Image::SummaryData data;
	DICOM::SummaryInfo info;
	bool result = files_view_->get_current_data( data, info);
	if (!result)
		return;

	Glib::RefPtr<Print::Operation> print = Print::Operation::create(info);

	print->set_track_print_status();
	print->set_default_page_setup(page_setup_);
	print->set_print_settings(print_settings_);
	if (action == Gtk::PRINT_OPERATION_ACTION_EXPORT)
		print->set_export_filename("/tmp/test_print.pdf");

	print->signal_done().connect(sigc::bind( sigc::mem_fun(
		*this, &MainWindow::on_printoperation_done), print));

	try {
		Gtk::PrintOperationResult res =
			(action == Gtk::PRINT_OPERATION_ACTION_EXPORT) ?
			print->run(action) : print->run( action, *this);
	}
	catch(const Gtk::PrintError& ex) {
		//See documentation for exact Gtk::PrintError error codes.
		std::cerr << "An error occurred while trying to run a print operation:"
			<< ex.what() << std::endl;
	}
}

MainWindow*
MainWindow::create(const Glib::OptionGroup::vecustrings& filenames)
{
	MainWindow* window = 0;
	Glib::RefPtr<Gtk::Builder> builder = create_from_file(builder_mainwindow_filename);

	// Get the GtkBuilder-instantiated window
	if (builder)
		builder->get_widget_derived( "main-window", window);

	if (window)
		window->files_ = filenames;

	return window; 
}

} // namespace UI

} // namespace ScanAmati
