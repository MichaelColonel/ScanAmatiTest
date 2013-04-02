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

#include "application.hpp"
#include "global_strings.hpp"

#include "utils.hpp"

#include "main_window.hpp"

namespace {

const char* const action_save = "action-file-save";
const char* const action_save_as = "action-file-save-as";
const char* const action_save_archive = "action-file-save-archive";
const char* const action_export_pdf = "action-export-pdf";
const char* const action_page_setup = "action-page-setup";
const char* const action_preview = "action-preview";
const char* const action_print = "action-print";
const char* const action_edit = "action-edit-dicom-info";

const char* const action_raw_data = "action-show-raw-data";
const char* const action_processing_data = "action-show-processing-data";
const char* const action_presentation_data = "action-show-presentation-data";
const char* const action_data_filter = "action-data-filter";
const char* const action_data_calibration = "action-data-calibration";

} // namespace

namespace ScanAmati {

namespace UI {

const ActionState image_actions[] = {
	{ action_save, true },
	{ action_save_as, true },
	{ action_save_archive, true },
	{ action_export_pdf, true },
	{ action_page_setup, true },
	{ action_preview, true },
	{ action_print, true },
	{ action_edit, true },
	{ } // Terminating Entry
};

const ActionState state_new_data_actions[] = {
	{ action_save, false },
	{ action_save_as, true },
	{ action_save_archive, true },
	{ action_export_pdf, true },
	{ action_page_setup, true },
	{ action_preview, true },
	{ action_print, true },
	{ action_edit, true },
	{ } // Terminating Entry
};

const ActionState state_data_changed_actions[] = {
	{ action_save, true },
	{ action_save_as, true },
	{ action_save_archive, true },
	{ action_export_pdf, true },
	{ action_page_setup, true },
	{ action_preview, true },
	{ action_print, true },
	{ action_edit, true },
	{ } // Terminating Entry
};

const ActionState state_raw_data_actions[] = {
	{ action_raw_data, true },
	{ action_processing_data, true },
	{ action_presentation_data, true },
	{ action_data_filter, true },
	{ action_data_calibration, true },
	{ action_edit, true },
	{ } // Terminating Entry
};

const ActionState state_processing_data_actions[] = {
	{ action_raw_data, false },
	{ action_processing_data, true },
	{ action_presentation_data, true },
	{ action_data_filter, false },
	{ action_data_calibration, false },
	{ action_edit, true },
	{ } // Terminating Entry
};

const ActionState state_presentation_data_actions[] = {
	{ action_raw_data, false },
	{ action_processing_data, false },
	{ action_presentation_data, false },
	{ action_data_filter, false },
	{ action_data_calibration, false },
	{ action_edit, true },
	{ } // Terminating Entry
};

void
MainWindow::on_images_cleaned()
{
	set_actions_state( image_actions, false);

	set_title(_("ScanAmati"));
}

void
MainWindow::on_file_view_state_type(FilesIconView::StateType type)
{
	switch (type) {
	case FilesIconView::STATE_NEW_DATA:
		set_actions_state(state_new_data_actions);
		break;
	case FilesIconView::STATE_FILE_DATA:
		set_actions_state(state_new_data_actions);
		break;
	case FilesIconView::STATE_FILE_SAVED:
		set_actions_state(state_new_data_actions);
		break;
	case FilesIconView::STATE_PACS_DATA:
		set_actions_state(state_new_data_actions);
		break;
	case FilesIconView::STATE_PACS_SAVED:
		set_actions_state(state_new_data_actions);
		break;
	case FilesIconView::STATE_CHANGED:
		set_actions_state(state_data_changed_actions);
		break;
	default:
		set_actions_state( state_data_changed_actions, false);
		break;
	}
}

void
MainWindow::on_file_view_data_type()
{

}

} // namespace UI

} // namespace ScanAmati
