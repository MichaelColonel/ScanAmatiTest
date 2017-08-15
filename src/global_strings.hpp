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

#include <glib.h> // for G_DIR_SEPARATOR_S
#include <config.h> // for SCANAMATI_PKGDATADIR

#include <paths.h> // for _PATH_DEV and _PATH_TMP

#include <glibmm/i18n.h>

#ifndef _PATH_DEV
#define _PATH_DEV "/dev/"
#endif

#ifndef _PATH_TMP
#define _PATH_TMP "/tmp/"
#endif

#include "scanner/defines.hpp"

namespace ScanAmati {

/**
 * The idea is taken from Daniel Elstner's Regexxer.
 * Thanks to the GNU compiler/linker, these namespaced string constants
 * are no less efficient than string literals or preprocessor #defines.
 * The final executable contains exactly one copy of each string literal,
 * so there's no need to define them in a separate object file.
 */

const char* const data_dir =
	SCANAMATI_PKGDATADIR G_DIR_SEPARATOR_S "data" G_DIR_SEPARATOR_S;

const char* const rc_dir = "scanamati";
const char* const rc_file = "configure";
const char* const temperature_file_extension = "temperature";
const char* const lining_file_extension = "lining";
const char* const bad_strips_file_extension = "bad_strips";
const char* const dicom_servers_file = "dicom_servers";
const char* const radiation_output_file = "radiation_output";

const char* const sound_caution_filename = SCANAMATI_PKGDATADIR
	G_DIR_SEPARATOR_S "sounds" G_DIR_SEPARATOR_S "caution.ogg";

namespace UI {

const char* const builder_mainwindow_filename = SCANAMATI_PKGDATADIR
	G_DIR_SEPARATOR_S "ui" G_DIR_SEPARATOR_S "main-window.glade";

const char* const builder_capacities_dialog_filename = SCANAMATI_PKGDATADIR
	G_DIR_SEPARATOR_S "ui" G_DIR_SEPARATOR_S "chip-capacities-dialog.glade";

const char* const builder_temperature_dialog_filename = SCANAMATI_PKGDATADIR
	G_DIR_SEPARATOR_S "ui" G_DIR_SEPARATOR_S "temperature-margins-dialog.glade";

const char* const builder_image_acquisition_dialog_filename = SCANAMATI_PKGDATADIR
	G_DIR_SEPARATOR_S "ui" G_DIR_SEPARATOR_S "image-acquisition-dialog.glade";

const char* const builder_image_acquisition_parameters_dialog_filename = SCANAMATI_PKGDATADIR
	G_DIR_SEPARATOR_S "ui" G_DIR_SEPARATOR_S "acquisition-parameters-dialog.glade";

const char* const builder_lining_acquisition_dialog_filename = SCANAMATI_PKGDATADIR
	G_DIR_SEPARATOR_S "ui" G_DIR_SEPARATOR_S "lining-acquisition-dialog.glade";

const char* const builder_lining_adjustment_dialog_filename = SCANAMATI_PKGDATADIR
	G_DIR_SEPARATOR_S "ui" G_DIR_SEPARATOR_S "lining-adjustment-dialog.glade";

const char* const builder_move_dialog_filename = SCANAMATI_PKGDATADIR
	G_DIR_SEPARATOR_S "ui" G_DIR_SEPARATOR_S "c-move-dialog.glade";

const char* const builder_store_dialog_filename = SCANAMATI_PKGDATADIR
	G_DIR_SEPARATOR_S "ui" G_DIR_SEPARATOR_S "c-store-dialog.glade";

const char* const builder_scanner_debug_dialog_filename = SCANAMATI_PKGDATADIR
	G_DIR_SEPARATOR_S "ui" G_DIR_SEPARATOR_S "scanner-debug-dialog.glade";

const char* const builder_movement_parameters_dialog_filename = SCANAMATI_PKGDATADIR
	G_DIR_SEPARATOR_S "ui" G_DIR_SEPARATOR_S "movement-parameters-dialog.glade";

const char* const builder_preferences_dialog_filename = SCANAMATI_PKGDATADIR
	G_DIR_SEPARATOR_S "ui" G_DIR_SEPARATOR_S "preferences-dialog.glade";

const char* const builder_dicom_server_dialog_filename = SCANAMATI_PKGDATADIR
	G_DIR_SEPARATOR_S "ui" G_DIR_SEPARATOR_S "dicom-server-dialog.glade";

const char* const builder_dicom_information_dialog_filename = SCANAMATI_PKGDATADIR
	G_DIR_SEPARATOR_S "ui" G_DIR_SEPARATOR_S "dicom-information-dialog.glade";

const char* const builder_conquest_archive_dialog_filename = SCANAMATI_PKGDATADIR
	G_DIR_SEPARATOR_S "ui" G_DIR_SEPARATOR_S "conquest-archive-dialog.glade";

const char* const builder_person_name_dialog_filename = SCANAMATI_PKGDATADIR
	G_DIR_SEPARATOR_S "ui" G_DIR_SEPARATOR_S "person-name-dialog.glade";

const char* const uimanager_menus_bars_filename = SCANAMATI_PKGDATADIR
	G_DIR_SEPARATOR_S "ui" G_DIR_SEPARATOR_S "menus-bars.ui";

const char* const conf_key_window_width = "window-width";
const char* const conf_key_window_height = "window-height";
const char* const conf_key_window_position_x = "window-position-x";
const char* const conf_key_window_position_y = "window-position-y";
const char* const conf_key_window_vpaned_position = "window-vpaned-position";
const char* const conf_key_window_hpaned_position = "window-hpaned-position";

// Dialog window configure templates
const char* const conf_key_dialog_width = "-dialog-width";
const char* const conf_key_dialog_height = "-dialog-height";
const char* const conf_key_dialog_position_x = "-dialog-position-x";
const char* const conf_key_dialog_position_y = "-dialog-position-y";

} // namespace UI

namespace Scanner {

const char* const conf_key_state_peltier_code = "peltier-code";
const char* const conf_key_state_chip_capacity = "chip-capacity";
const char* const conf_key_state_temperature_control = "temperature-control";
const char* const conf_key_state_temperature_average = "temperature-average";
const char* const conf_key_state_temperature_spread = "temperature-spread";

const char array_chip_codes[SCANNER_CHIPS] = {
#if SCANNER_CHIPS == 16
	'0', '1', '2', '3',
	'4', '5', '6', '7',
	'8', '9', 'A', 'B',
	'C', 'D', 'E', 'F'
#elif SCANNER_CHIPS == 12
	'2', '3', '4', '5',
	'6', '7', '8', '9',
	'A', 'B', 'C', 'D'
#endif
};

const char* const handshake_message = "Welcome";
const char* const OK_message = "OK";
const char* const id_template = N_("APRM");
const char* const id_default = "APRMXXX";

// "/dev/xray_scanner"
const char* const device_name = _PATH_DEV "xray_scanner";

} // namespace Scanner

const char* const filter_dcm_name = N_("DICOM image (*.dcm)");
const char* const filter_raw_name = N_("Studio raw file (*.raw)");
const char* const filter_jpg_name = N_("JPEG image file (*.jpg, *jpeg)");
const char* const filter_png_name = N_("PNG image file (*.png)");
const char* const dicom_tmp_file = _PATH_TMP "scanamati_XXXXXX.dcm";

const char* const dot_pattern = "\\.";

} // namespace ScanAmati
