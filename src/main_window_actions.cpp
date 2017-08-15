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
#include <gtkmm/menu.h>
#include <glibmm/i18n.h>

#include "palette/palette.hpp"
#include "widgets/files_icon_view.hpp"
#include "widgets/image_area.hpp"
#include "widgets/palette_area.hpp"
#include "widgets/scrolled_image_area.hpp"

#include "application.hpp"

#include "main_window.hpp"

namespace ScanAmati {

namespace UI {

/**
 * Initializes action group and all actions.
 */
void
MainWindow::init_actions()
{
	Glib::RefPtr<Gtk::Action> act;
	Glib::RefPtr<Gtk::RadioAction> radio_act;

	action_group_ = Gtk::ActionGroup::create();
	// File menu
	action_group_->add(Gtk::Action::create( "action-file", _("_File")));

	act = Gtk::Action::create( "action-file-open", Gtk::Stock::OPEN,
		_("_Open..."), _("Open an existing image"));
	action_group_->add( act, sigc::mem_fun( *this, &MainWindow::on_file_open));

	act = Gtk::Action::create( "action-file-save", Gtk::Stock::SAVE, _("_Save"),
		_("Save image"));
	action_group_->add(act);
	act->set_sensitive(false);

	act = Gtk::Action::create( "action-file-save-as", Gtk::Stock::SAVE_AS,
		_("Save _As..."), _("Save image under a new name"));
	action_group_->add( act,
		sigc::mem_fun( *this, &MainWindow::on_file_save_as));
	act->set_sensitive(false);

	act = Gtk::Action::create( "action-file-save-archive", Gtk::Stock::SAVE_AS,
		_("_Save to Archive"), _("Save image to archive"));
	action_group_->add( act,
		sigc::mem_fun( *this, &MainWindow::on_file_save_archive));
	act->set_sensitive(false);

	act = Gtk::Action::create( "preferences", Gtk::Stock::PREFERENCES,
		_("ScanAmati Preferences..."), _("Edit global ScanAmati preferences"));
	action_group_->add( act, Gtk::AccelKey("<control><shift>p"),
		sigc::mem_fun( *this, &MainWindow::on_preferences));

	act = Gtk::Action::create( "action-page-setup", Gtk::Stock::PAGE_SETUP);
	action_group_->add( act,
		sigc::mem_fun( *this, &MainWindow::on_print_page_setup));
	act->set_sensitive(false);

	act = Gtk::Action::create( "action-preview", Gtk::Stock::PRINT_PREVIEW);
	action_group_->add( act, sigc::bind(
		sigc::mem_fun( *this, &MainWindow::on_print_preview),
		Gtk::PRINT_OPERATION_ACTION_PREVIEW));
	act->set_sensitive(false);

	act = Gtk::Action::create( "action-print", Gtk::Stock::PRINT);
	action_group_->add( act, Gtk::AccelKey("<control>p"), sigc::bind(
		sigc::mem_fun( *this, &MainWindow::on_print_preview),
		Gtk::PRINT_OPERATION_ACTION_PRINT_DIALOG));
	act->set_sensitive(false);

	act = Gtk::Action::create( "action-export-pdf", Gtk::StockID(),
		_("Export PDF..."));
	action_group_->add( act, sigc::bind(
		sigc::mem_fun( *this, &MainWindow::on_print_preview),
		Gtk::PRINT_OPERATION_ACTION_EXPORT));
	act->set_sensitive(false);

	act = Gtk::Action::create( "quit", Gtk::Stock::QUIT);
	action_group_->add( act, sigc::mem_fun( *this, &MainWindow::on_quit));

	act = Gtk::Action::create( "action-image-find", Gtk::Stock::FIND,
		_("_Find Image..."), _("Find Image In The Localhost Conquest DICOM Server"));
	action_group_->add( act, Gtk::AccelKey("<control>f"),
		sigc::mem_fun( *this, &MainWindow::on_image_find));

	act = Gtk::Action::create( "action-edit", _("_Edit"));
	action_group_->add(act);

	act = Gtk::Action::create( "action-edit-dicom-info", Gtk::Stock::EDIT,
		_("Edit DICOM _Information..."));
	action_group_->add( act,
		sigc::mem_fun( *this, &MainWindow::on_edit_dicom_info));
	act->set_sensitive(false);

	act = Gtk::Action::create( "action-new-dicom-info", Gtk::Stock::NEW,
		_("_New DICOM Information..."));
	action_group_->add( act,
		sigc::mem_fun( *this, &MainWindow::on_new_dicom_info));

	act = Gtk::Action::create( "action-image", _("_Image"));
	action_group_->add(act);

	act = Gtk::Action::create( "action-palette", _("_Palette"));
	action_group_->add(act);

	Gtk::RadioButtonGroup group_palette; // Image Palettes
	radio_act = Gtk::RadioAction::create( group_palette,
		"radioaction-palette-grayscale", Q_("Palette|Grayscale"));
	action_group_->add( radio_act, sigc::bind(
		sigc::mem_fun( *palette_area_,
			&PaletteArea::on_palette_type_changed),
		radio_act, Image::PALETTE_GRAYSCALE));
	radio_act->set_active(true);

	radio_act = Gtk::RadioAction::create( group_palette,
		"radioaction-palette-inverted", Q_("Palette|Inverted"));
	action_group_->add( radio_act, sigc::bind(
		sigc::mem_fun( *palette_area_,
			&PaletteArea::on_palette_type_changed),
		radio_act, Image::PALETTE_INVERTED));

	radio_act = Gtk::RadioAction::create( group_palette,
		"radioaction-palette-rainbow", Q_("Palette|Rainbow"));
	action_group_->add( radio_act, sigc::bind(
		sigc::mem_fun( *palette_area_,
			&PaletteArea::on_palette_type_changed),
		radio_act, Image::PALETTE_RAINBOW));

	radio_act = Gtk::RadioAction::create( group_palette,
		"radioaction-palette-combined", Q_("Palette|Combined"));
	action_group_->add( radio_act, sigc::bind(
		sigc::mem_fun( *palette_area_,
			&PaletteArea::on_palette_type_changed),
		radio_act, Image::PALETTE_COMBINED));

	radio_act = Gtk::RadioAction::create( group_palette,
		"radioaction-palette-hotmetal", Q_("Palette|Hot metal"));
	action_group_->add( radio_act, sigc::bind(
		sigc::mem_fun( *palette_area_,
			&PaletteArea::on_palette_type_changed),
		radio_act, Image::PALETTE_HOTMETAL));

	act = Gtk::Action::create( "action-zoom", _("_Zoom"));
	action_group_->add(act);

	act = Gtk::Action::create( "action-zoom-width", Gtk::Stock::ZOOM_FIT,
		_("Image _Width"), _("Zoom to fit image width in window"));
	action_group_->add( act, Gtk::AccelKey("<control>2"),
		sigc::bind( sigc::mem_fun( *image_area_, &ImageArea::on_zoom),
		ZOOM_WIDTH));

	act = Gtk::Action::create( "action-zoom-height", Gtk::Stock::ZOOM_FIT,
		_("Image _Heigth"), _("Zoom to fit image height in window"));
	action_group_->add( act, Gtk::AccelKey("<control>3"),
		sigc::bind( sigc::mem_fun( *image_area_, &ImageArea::on_zoom),
		ZOOM_HEIGHT));

	act = Gtk::Action::create( "action-zoom-100", Gtk::StockID(),
		_("Zoom _1:1 (100%)"), _("Zoom to 1:1"));
	action_group_->add( act, Gtk::AccelKey("<control>1"),
		sigc::bind( sigc::mem_fun( *image_area_, &ImageArea::on_zoom),
		ZOOM_100));

	act = Gtk::Action::create( "action-zoom-200", Gtk::StockID(),
		_("Zoom _2:1 (200%)"), _("Zoom to 2:1"));
	action_group_->add( act, sigc::bind(
		sigc::mem_fun( *image_area_, &ImageArea::on_zoom), ZOOM_200));

	act = Gtk::Action::create( "action-zoom-300", Gtk::StockID(),
		_("Zoom _3:1 (300%)"), _("Zoom to 3:1"));
	action_group_->add( act, sigc::bind(
		sigc::mem_fun( *image_area_, &ImageArea::on_zoom), ZOOM_300));

	Glib::RefPtr<Gtk::ToggleAction> toggle_action = Gtk::ToggleAction::create(
		"action-draw-margins", Gtk::StockID(), _("Draw _Margins"),
		_("Show detector's margins on an image"));

	action_group_->add( toggle_action, sigc::bind(
		sigc::mem_fun( *image_area_, &ImageArea::on_draw_margins),
		toggle_action));

	toggle_action->set_sensitive(app.extend);

	toggle_action = Gtk::ToggleAction::create(
		"action-draw-broken-strips", Gtk::StockID(), _("Draw _Broken Strips"),
		_("Show broken strips on the image"));

	action_group_->add( toggle_action, sigc::bind(
		sigc::mem_fun( image_area_, &ImageArea::on_draw_broken_strips),
		toggle_action));

	toggle_action->set_sensitive(app.extend);

	act = Gtk::Action::create( "action-scanner", _("_Scanner"));
	action_group_->add(act);

	act = Gtk::Action::create( "action-scanner-device", _("_Device"));
	action_group_->add(act);

	act = Gtk::Action::create( "action-device-connect", Gtk::Stock::CONNECT,
		Q_("Device|_Connect"), _("Connect Scanner Device"));
	action_group_->add( act, Gtk::AccelKey("<shift><control>c"),
		sigc::bind( sigc::mem_fun( *this, &MainWindow::on_scanner_device), true));

	act = Gtk::Action::create( "action-device-disconnect", Gtk::Stock::DISCONNECT,
		Q_("Device|_Disconnect"), _("Disconnect Scanner Device"));
	action_group_->add( act, Gtk::AccelKey("<shift><control>d"),
		sigc::bind( sigc::mem_fun( *this, &MainWindow::on_scanner_device), false));
	act->set_sensitive(false);

	act = Gtk::Action::create( "action-lining-acquisition", Gtk::StockID(),
		_("_Lining Procedure..."), _("Acquire lining coefficients"));
	action_group_->add( act, Gtk::AccelKey("<control>l"),
		sigc::mem_fun( *this, &MainWindow::on_lining_acquisition));
	act->set_sensitive(false);

	act = Gtk::Action::create( "action-image-acquisition", Gtk::Stock::MEDIA_PLAY,
		_("Run the _Acquisition..."), _("Acquire a x-ray image"));
	action_group_->add( act, Gtk::AccelKey("<control>a"),
		sigc::mem_fun( *this, &MainWindow::on_image_acquisition));
	act->set_sensitive(false);

	act = Gtk::Action::create( "action-temperature-margins", Gtk::StockID(),
		_("Temperature _Margins..."));
	action_group_->add( act, sigc::mem_fun( *this, &MainWindow::on_temperature_margins));
	act->set_sensitive(false);

	act = Gtk::Action::create( "action-scanner-debug", Gtk::StockID(),
		_("Debug..."));
	action_group_->add( act, sigc::mem_fun( *this, &MainWindow::on_scanner_debug));
	act->set_sensitive(false);

	act = Gtk::Action::create( "action-remove-current",
		Gtk::Stock::DELETE, _("Remove Current"));
	action_group_->add( act, sigc::mem_fun( *files_view_, &FilesIconView::remove_current));

	act = Gtk::Action::create( "action-files-clear-all", Gtk::StockID(),
		_("Clear All"));
	action_group_->add( act, sigc::mem_fun( *files_view_, &FilesIconView::clear_all));

	act = Gtk::Action::create( "action-show-raw-data", Gtk::StockID(),
		_("Show Raw Data"));
	action_group_->add(act);

	act = Gtk::Action::create( "action-show-processing-data", Gtk::StockID(),
		_("Show Processing Data"));
	action_group_->add(act);

	act = Gtk::Action::create( "action-show-presentation-data", Gtk::StockID(),
		_("Show Presentation Data"));
	action_group_->add(act);
	
	action_group_->add(Gtk::Action::create( "action-data-filter",
		_("Data _Filter")));

	act = Gtk::Action::create( "action-filter-lanczos", Gtk::StockID(),
		_("Lanczos"));
	action_group_->add(act);

	act = Gtk::Action::create( "action-filter-cubic", Gtk::StockID(),
		_("Cubic"));
	action_group_->add(act);

	act = Gtk::Action::create( "action-filter-bessel", Gtk::StockID(),
		_("Bessel"));
	action_group_->add(act);

	act = Gtk::Action::create( "action-filter-quadratic", Gtk::StockID(),
		_("Quadratic"));
	action_group_->add(act);

	action_group_->add(Gtk::Action::create( "action-data-calibration",
		_("Data _Calibration")));

	act = Gtk::Action::create( "action-calibration-rough", Gtk::StockID(),
		_("Rough"));
	action_group_->add(act);

	act = Gtk::Action::create( "action-calibration-good", Gtk::StockID(),
		_("Good"));
	action_group_->add(act);

	act = Gtk::Action::create( "action-calibration-better", Gtk::StockID(),
		_("Better"));
	action_group_->add(act);

	act = Gtk::Action::create( "action-calibration-best", Gtk::StockID(),
		_("Best"));
	action_group_->add(act);

	// popup menus
	act = Gtk::Action::create( "popup-menu-palette", _("_Palette"));
	action_group_->add(act);

	act = Gtk::Action::create( "popup-files-view", _("_Files"));
	action_group_->add(act);

	act = Gtk::Action::create( "popup-file-icons", _("File _Icons"));
	action_group_->add(act);

	act = Gtk::Action::create( "popup-image-area", _("Image _Area"));
	action_group_->add(act);

	ui_manager_->insert_action_group(action_group_);
	add_accel_group(ui_manager_->get_accel_group());
}

} // namespace UI

} // namespace ScanAmati
