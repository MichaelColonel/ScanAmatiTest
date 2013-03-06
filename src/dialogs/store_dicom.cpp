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
#include "dicom/utils.hpp"
// files from src directory end

#include "utils.hpp"

#include "store_dicom.hpp"

namespace {

const struct ModelColumns : public Gtk::TreeModel::ColumnRecord {
	ModelColumns() { add(label); add(id); }

	Gtk::TreeModelColumn<Glib::ustring> label;
	Gtk::TreeModelColumn<Glib::ustring> id;
} model_columns;

ScanAmati::UI::StoreDicomDialog* dialog_ptr;

void
progress( void* data, T_DIMSE_StoreProgress* progress,
	T_DIMSE_C_StoreRQ* request)
{
	dialog_ptr->store_progress( data, progress, request);
}

DIMSE_StoreUserCallback progress_callback = &progress;

const char* conquest_unique_name = N_("\"Conquest\" - localhost DICOM server");

} // namespace

namespace ScanAmati {

namespace UI {

StoreDicomDialog::StoreDicomDialog( BaseObjectType* cobject,
	const Glib::RefPtr<Gtk::Builder>& builder)
	:
	Gtk::Dialog(cobject),
	builder_(builder),
	progressbar_(0),
	button_store_(0),
	combobox_servers_(0),
	textview_status_(0),
	textview_errors_(0),
	offset_status_(0),
	offset_errors_(0),
	dataset_(0)
{
	init_ui();

	connect_signals();

	fill_servers_liststore();

	combobox_servers_->set_active(0);

	load_preferences();

	show_all_children();
}

StoreDicomDialog::~StoreDicomDialog()
{
	save_preferences();
}

void
StoreDicomDialog::init_ui()
{
	// Get the Glade-instantiated widgets.
	builder_->get_widget( "combobox-servers", combobox_servers_);

	Glib::RefPtr<Glib::Object> obj = builder_->get_object("liststore-servers");
	liststore_servers_ = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(obj);

	builder_->get_widget( "button-store", button_store_);
	builder_->get_widget( "progressbar", progressbar_);
	builder_->get_widget( "textview-errors", textview_errors_);
	builder_->get_widget( "textview-status", textview_status_);

	Glib::ustring dir = app.prefs.get<Glib::ustring>( "ConQuest",
		"localhost-server-directory");

	DICOM::ConquestFiles conquest(dir.raw());

	// Connect logs files
	Glib::RefPtr<Gio::File> file =
		Gio::File::create_for_path(conquest.log_file());
	filemonitor_status_ = file->monitor_file();
	filestream_status_ = file->read();
	filestream_status_->seek( 0, Glib::SEEK_TYPE_END);
	offset_status_ = filestream_status_->tell();

	file = Gio::File::create_for_path(conquest.error_file());
	filemonitor_errors_ = file->monitor_file();
	filestream_errors_ = file->read();
	filestream_errors_->seek( 0, Glib::SEEK_TYPE_END);
	offset_errors_ = filestream_errors_->tell();
}

void
StoreDicomDialog::load_preferences()
{
	int h = app.prefs.get<int>( "Gui", "store-dicom-dialog-height");
	int w = app.prefs.get<int>( "Gui", "store-dicom-dialog-width");

	resize( w, h);
}

void
StoreDicomDialog::save_preferences()
{
	int w, h;

	get_size( w, h);

	app.prefs.set( "Gui", "store-dicom-dialog-height", h);
	app.prefs.set( "Gui", "store-dicom-dialog-width", w);
}

void
StoreDicomDialog::connect_signals()
{
	combobox_servers_->signal_changed().connect(sigc::mem_fun(
		*this, &StoreDicomDialog::on_dicom_server_changed));
	button_store_->signal_clicked().connect(sigc::mem_fun(
		*this, &StoreDicomDialog::on_store));

	if (textview_status_)
		filemonitor_status_->signal_changed().connect(
			sigc::mem_fun( *this, &StoreDicomDialog::on_status_changed));
	if (textview_errors_)
		filemonitor_errors_->signal_changed().connect(
			sigc::mem_fun( *this, &StoreDicomDialog::on_errors_changed));
}

void
StoreDicomDialog::on_status_changed( const Glib::RefPtr<Gio::File>& file,
	const Glib::RefPtr<Gio::File>&,
	Gio::FileMonitorEvent event)
{
	switch (event) {
	case Gio::FILE_MONITOR_EVENT_CHANGED:
		{
			filestream_status_->seek( 0, Glib::SEEK_TYPE_END);
			goffset pos = filestream_status_->tell();
			int diff = pos - offset_status_;
			if (diff > 0) {
				char* data = new char[diff];
				filestream_status_->seek( offset_status_, Glib::SEEK_TYPE_SET);
				filestream_status_->read( data, diff);
				Glib::RefPtr<Gtk::TextBuffer> buf = Gtk::TextBuffer::create();
				buf->assign( data, data + diff);
				textview_status_->set_buffer(buf);
				delete [] data;
			}
			else
				break;
		}
		break;
	case Gio::FILE_MONITOR_EVENT_CHANGES_DONE_HINT:
		filestream_status_->seek( 0, Glib::SEEK_TYPE_END);
		offset_status_ = filestream_status_->tell();
		break;
	default:
		break;
	}
}

void
StoreDicomDialog::on_errors_changed( const Glib::RefPtr<Gio::File>& file,
	const Glib::RefPtr<Gio::File>&,
	Gio::FileMonitorEvent event)
{
	switch (event) {
	case Gio::FILE_MONITOR_EVENT_CHANGED:
		{
			filestream_errors_->seek( 0, Glib::SEEK_TYPE_END);
			goffset pos = filestream_errors_->tell();
			int diff = pos - offset_errors_;
			if (diff > 0) {
				char* data = new char[diff];
				filestream_errors_->seek( offset_errors_, Glib::SEEK_TYPE_SET);
				filestream_errors_->read( data, diff);
				Glib::RefPtr<Gtk::TextBuffer> buf = Gtk::TextBuffer::create();
				buf->assign( data, data + diff);
				textview_errors_->set_buffer(buf);
				delete [] data;
			}
			else
				break;
		}
		break;
	case Gio::FILE_MONITOR_EVENT_CHANGES_DONE_HINT:
		filestream_errors_->seek( 0, Glib::SEEK_TYPE_END);
		offset_errors_ = filestream_errors_->tell();
		break;
	default:
		break;
	}
}

void
StoreDicomDialog::on_store()
{
	DICOM::Server server;
	Gtk::TreeIter iter = combobox_servers_->get_active();
	if (iter) {
		Gtk::TreeModel::Row row = *iter;
		Glib::ustring id = row[model_columns.id];
		if (id == gettext(conquest_unique_name)) {

			Glib::ustring dir = app.prefs.get<Glib::ustring>(
				"ConQuest",	"localhost-server-directory");
			DICOM::ConquestFiles conquest(dir.raw());
			DICOM::ConquestSettings settings = conquest.get_settings();

			server.title = settings.title();
			server.host = "localhost";
			server.port = settings.port();
		}
		else
			server = app.dicom_servers[id];
	}

	try {
		DICOM::StoreCommand store(server);
		bool res = store.run( dataset_, progress_callback);
	}
	catch (const Exception& ex) {
		std::cout << ex.what() << std::endl;
	}
}

void
StoreDicomDialog::on_response(int)
{
	hide();
}

void
StoreDicomDialog::on_dicom_server_changed()
{
	int i = combobox_servers_->get_active_row_number();
	Gtk::Widget* expander;
	builder_->get_widget( "expander-log", expander);
	if (expander)
		expander->set_sensitive(i == 0);
}

void
StoreDicomDialog::fill_servers_liststore()
{
	Glib::ustring dir = app.prefs.get<Glib::ustring>( "ConQuest",
		"localhost-server-directory");

	DICOM::ConquestFiles conquest(dir.raw());
	DICOM::ConquestSettings settings = conquest.get_settings();

	Gtk::TreeModel::Row row = *(liststore_servers_->append());

	Glib::ustring label = Glib::ustring::compose(
		_("<b>%1</b>\n(%2) %3:%4"), gettext(conquest_unique_name),
		settings.title(), "localhost",
		Glib::ustring::format(settings.port()));

	row[model_columns.label] = label;
	row[model_columns.id] = gettext(conquest_unique_name);

	DICOM::ServersMap& map = app.dicom_servers;
	for ( DICOM::ServersMap::const_iterator iter = map.begin();
		iter != map.end(); ++iter) {
		row = *(liststore_servers_->append());

		label = Glib::ustring::compose(
			"<b>%1</b>\n(%2) %3", iter->first, iter->second.title,
				iter->second.called_address());

		row[model_columns.label] = label;
		row[model_columns.id] = iter->first;
	}
}

void
StoreDicomDialog::store_progress( void* data,
	T_DIMSE_StoreProgress* prog,
	T_DIMSE_C_StoreRQ* request)
{
	switch (prog->state) {
	case DIMSE_StoreBegin:
		progressbar_->set_fraction(0.);
		break;
	case DIMSE_StoreEnd:
		progressbar_->set_fraction(1.);
		progressbar_->set_text(_("File has been stored successfully!"));
		break;
	default:
		{
			double fraction = double(prog->progressBytes) / prog->totalBytes;
			if (fraction > 1.)
				fraction = 1.;

			progressbar_->set_fraction(fraction);
			Glib::ustring text = Glib::ustring::compose(
				_("Have been written %1 of %2 bytes."),
				Glib::ustring::format(prog->progressBytes),
				Glib::ustring::format(prog->totalBytes));
			progressbar_->set_text(text);
		}
		while (Gtk::Main::events_pending())
			Gtk::Main::iteration();
		break;
	}
}

StoreDicomDialog*
StoreDicomDialog::create(DICOM::Dataset* dataset)
{
	StoreDicomDialog* dialog = 0;

	std::string builder_file(builder_store_dialog_filename);
	Glib::RefPtr<Gtk::Builder> builder = create_from_file(builder_file);

	if (builder)
		//Get the GtkBuilder-instantiated dialog
		builder->get_widget_derived( "dialog-window", dialog);

	if (dialog) {
		dialog->dataset_ = dataset;
		dialog_ptr = dialog;
	}

	return dialog;
}

} // namespace UI

} // namespace ScanAmati
