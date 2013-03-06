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

#include <gtkmm/combobox.h>
#include <gtkmm/progressbar.h>
#include <gtkmm/main.h>
#include <gtkmm/textbuffer.h>
#include <gtkmm/textview.h>

#include <giomm/file.h>

#include <glibmm/i18n.h>

// files from src directory begin
#include "application.hpp"
#include "preferences.hpp"
#include "exceptions.hpp"

#include "global_strings.hpp"

#include "dicom/conquest.hpp"
#include "dicom/server.hpp"
#include "dicom/user_commands.hpp"
// files from src directory end

#include "utils.hpp"

#include "move_dicom.hpp"

namespace {

ScanAmati::UI::MoveDicomDialog* dialog_ptr;

void
progress( void* callback_data, T_DIMSE_StoreProgress* prog,
	T_DIMSE_C_StoreRQ* req, char* file, DcmDataset** image,
	T_DIMSE_C_StoreRSP* response, DcmDataset** status)
{
	dialog_ptr->move_progress( callback_data, prog, req, file, image,
		response, status);
}

DIMSE_StoreProviderCallback progress_callback = &progress;

} // namespace

namespace ScanAmati {

namespace UI {

MoveDicomDialog::MoveDicomDialog( BaseObjectType* cobject,
	const Glib::RefPtr<Gtk::Builder>& builder)
	:
	Gtk::Dialog(cobject),
	builder_(builder),
	progressbar_(0)
{
	init_ui();

	connect_signals();

	show_all_children();
}

MoveDicomDialog::~MoveDicomDialog()
{
}

void
MoveDicomDialog::init_ui()
{
	builder_->get_widget( "progressbar", progressbar_);
}

void
MoveDicomDialog::connect_signals()
{
}

void
MoveDicomDialog::move_query( const DcmDataset& dataset,
	const std::string& directory)
{
	directory_ = directory;
	Glib::signal_idle().connect(sigc::bind_return( sigc::bind( sigc::mem_fun(
		*this, &MoveDicomDialog::query), dataset), false));
}

void
MoveDicomDialog::query(const DcmDataset& query)
{
	Glib::ustring dir = app.prefs.get<Glib::ustring>( "ConQuest",
		"localhost-server-directory");

	DICOM::ConquestFiles cfiles(dir.raw());
	DICOM::ConquestSettings csettings = cfiles.get_settings();

	DICOM::Server server;
	server.host = "localhost";
	server.title = csettings.title();
	server.port = csettings.port();

	try {
		DICOM::MoveCommand comm(server);
		bool res = comm.run( query, directory_.c_str(), progress_callback);
		signal_move_result_( res, message_);
	}
	catch (const Exception& ex) {
		signal_move_result_( false, ex.what());
	}
}

void
MoveDicomDialog::on_response(int)
{
	hide();
}

void
MoveDicomDialog::move_progress( void* callback_data,
	T_DIMSE_StoreProgress* prog, T_DIMSE_C_StoreRQ* req, char* file,
	DcmDataset** image, T_DIMSE_C_StoreRSP* response, DcmDataset** status)
{
	switch (prog->state) {
	case DIMSE_StoreBegin:
		progressbar_->set_fraction(0.);
		break;
	case DIMSE_StoreEnd:
		progressbar_->set_fraction(1.);
		progressbar_->set_text(_("File has been moved successfully!"));
		break;
	default:
		{
			double fraction = double(prog->progressBytes) / prog->totalBytes;
			progressbar_->set_fraction(fraction);
			Glib::ustring text = Glib::ustring::compose( "Have been read %1 of %2 bytes.",
				Glib::ustring::format(prog->progressBytes),
				Glib::ustring::format(prog->totalBytes));
			progressbar_->set_text(text);
		}
		while (Gtk::Main::events_pending())
			Gtk::Main::iteration();
		break;
	}

	if (prog->state == DIMSE_StoreEnd) {
		*status = NULL;

		if ((image != NULL) && (*image != NULL)) {
			DICOM::UserCommand::StoreCallbackData* cbdata =
				static_cast<DICOM::UserCommand::StoreCallbackData*>(callback_data);

			E_TransferSyntax xfer = (*image)->getOriginalXfer();

			// create full path name for the output file
			OFString ofname;
			OFStandard::combineDirAndFilename( ofname, directory_.c_str(),
				cbdata->imageFileName, OFTrue);

			OFCondition cond = cbdata->dcmff->saveFile( ofname.c_str(), xfer,
				EET_ExplicitLength, EGL_withGL, EPD_withoutPadding, 0, 0,
				EWM_fileformat);

			if (cond.bad()) {
				OFLOG_DEBUG( app.log, "Cannot write DICOM file: " << ofname);
				response->DimseStatus = STATUS_STORE_Refused_OutOfResources;
				message_ = Glib::ustring(_("Cannot write DICOM file."));
			}
			else
				message_ = Glib::ustring(ofname.c_str());
			if (response->DimseStatus == STATUS_Success) {
				DIC_UI sopClass;
				DIC_UI sopInstance;
				// which SOP class and SOP instance ?
				if (!DU_findSOPClassAndInstanceInDataSet( *image,
					sopClass, sopInstance, OFFalse)) {

					OFLOG_DEBUG( app.log, "Bad DICOM file: " << image);
					response->DimseStatus = STATUS_STORE_Error_CannotUnderstand;
					message_ = Glib::ustring(_("Bad DICOM file."));
				}
				else if (strcmp( sopClass, req->AffectedSOPClassUID) != 0) {
					response->DimseStatus = STATUS_STORE_Error_DataSetDoesNotMatchSOPClass;
					message_ = Glib::ustring(_("Data doesn't match SOP class IUD."));
				}
				else if (strcmp( sopInstance, req->AffectedSOPInstanceUID) != 0) {
					response->DimseStatus = STATUS_STORE_Error_DataSetDoesNotMatchSOPClass;
					message_ = Glib::ustring(_("Data doesn't match SOP instance IUD."));
				}
			}
		}
    }
}

sigc::signal< void, bool, const Glib::ustring&>
MoveDicomDialog::signal_move_result()
{
	return signal_move_result_;
}

MoveDicomDialog*
MoveDicomDialog::create()
{
	MoveDicomDialog* dialog = 0;

	std::string filename(builder_move_dialog_filename);
	Glib::RefPtr<Gtk::Builder> builder = create_from_file(filename);

	if (builder)
		//Get the GtkBuilder-instantiated dialog
		builder->get_widget_derived( "dialog-window", dialog);

	dialog_ptr = dialog;

	return dialog;
}

} // namespace UI

} // namespace ScanAmati
