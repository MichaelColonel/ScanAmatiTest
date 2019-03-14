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

#include <pqxx/transaction>

#include <gtkmm/treeview.h>
#include <glibmm/i18n.h>

// files from src directory begin
#include "application.hpp"
#include "global_strings.hpp" // constant strings, filenames, paths

#include "dicom/conquest.hpp"
#include "dicom/short_information.hpp"
#include "dicom/utils.hpp"
// files from src directory end

#include "move_dicom.hpp"

#include "utils.hpp"

#include "conquest_archive.hpp"

namespace {

typedef std::list<Gtk::TreeViewColumn*>::iterator ColumnsIter;

const struct ModelColumns : public Gtk::TreeModel::ColumnRecord {
	Gtk::TreeModelColumn<Glib::ustring> study_id;
	Gtk::TreeModelColumn<Glib::ustring> study_descr;
	Gtk::TreeModelColumn<Glib::ustring> study_date;
	Gtk::TreeModelColumn<Glib::ustring> study_time;
	Gtk::TreeModelColumn<Glib::ustring> patient_id;
	Gtk::TreeModelColumn<Glib::ustring> patient_name;
	Gtk::TreeModelColumn<Glib::ustring> patient_birthday;
	Gtk::TreeModelColumn<Glib::ustring> patient_sex;
	Gtk::TreeModelColumn<std::string> study_instance_uid;
	Gtk::TreeModelColumn<bool> visible;
	ModelColumns() {
		add(study_id);
		add(study_descr);
		add(study_date);
		add(study_time);
		add(patient_id);
		add(patient_name);
		add(patient_birthday);
		add(patient_sex);
		add(study_instance_uid);
		add(visible);
	}
} model_columns;

const char* conninfo = "dbname = 'conquest' user = 'postgres'";

const char* prepared1 = "query_case_insense";

const char* query1 =
	"SELECT p.patientid, t.studyid, t.studydate, p.patientnam, p.patientsex, "
	"p.patientbir, t.studydescr, t.studytime, t.studyinsta "
		"FROM dicompatients p, dicomstudies t, dicomseries s, dicomimages i "
			"WHERE (p.patientid = t.patientid) "
			"AND (t.studyinsta = s.studyinsta) "
			"AND (s.seriesinst = i.seriesinst) "
			"AND (p.patientnam ~* $1) "
			"AND (p.patientid ~* $2) "
			"AND (t.studyid ~* $3) "
			"ORDER BY p.patientid";

const char* prepared2 = "query_case_sense";

const char* query2 =
	"SELECT p.patientid, t.studyid, t.studydate, p.patientnam, p.patientsex, "
	"p.patientbir, t.studydescr, t.studytime, t.studyinsta "
		"FROM dicompatients p, dicomstudies t, dicomseries s, dicomimages i "
			"WHERE (p.patientid = t.patientid) "
			"AND (t.studyinsta = s.studyinsta) "
			"AND (s.seriesinst = i.seriesinst) "
			"AND (p.patientnam ~ $1) "
			"AND (p.patientid ~ $2) "
			"AND (t.studyid ~ $3) "
			"ORDER BY p.patientid";

bool
compare_ids( pqxx::tuple i, pqxx::tuple j)
{
	return (i[0].as(std::string()) != j[0].as(std::string()));
}

class ConfirmationDialog : public Gtk::MessageDialog {
public:
	ConfirmationDialog(Gtk::Window& parent)
		:
		Gtk::MessageDialog( parent,
			_("Move selected image?"),
			false,
			Gtk::MESSAGE_QUESTION,
			Gtk::BUTTONS_YES_NO)
	{
		set_title(_("The query confirmation"));
		set_secondary_text(_("If you want to move selected image\n"
			"from the server press \"Yes\" otherwise press \"No\"."));
	}
};

}; // namespace

namespace ScanAmati {

namespace UI {

struct ArchiveItem {
	ArchiveItem();
	ArchiveItem( const Glib::ustring&,
		const std::vector<ArchiveItem>&);
	ArchiveItem(const ArchiveItem&);
	~ArchiveItem();
	ArchiveItem& operator=(const ArchiveItem&);

	Glib::ustring study_id;
	Glib::ustring study_descr;
	Glib::Date study_date;
	Glib::ustring study_time;
	Glib::ustring patient_id;
	Glib::ustring patient_name;
	Glib::Date patient_birthday;
	Glib::ustring patient_sex;
	std::string study_instance_uid;
	std::vector<ArchiveItem> children;
};

ArchiveItem::ArchiveItem()
	:
	study_date( 1, Glib::Date::JANUARY, 1900),
	patient_birthday( 1, Glib::Date::JANUARY, 1900)
{
}

ArchiveItem::ArchiveItem( const Glib::ustring& id,
	const std::vector<ArchiveItem>& children_items)
	:
	study_id(id),
	study_date( 1, Glib::Date::JANUARY, 1900),
	patient_birthday( 1, Glib::Date::JANUARY, 1900),
	children(children_items)
{
}

ArchiveItem::ArchiveItem(const ArchiveItem& foo)
	:
	study_id(foo.study_id),
	study_descr(foo.study_descr),
	study_date(foo.study_date),
	study_time(foo.study_time),
	patient_id(foo.patient_id),
	patient_name(foo.patient_name),
	patient_birthday(foo.patient_birthday),
	patient_sex(foo.patient_sex),
	study_instance_uid(foo.study_instance_uid),
	children(foo.children)
{
}

ArchiveItem::~ArchiveItem()
{
}

ArchiveItem&
ArchiveItem::operator=(const ArchiveItem& foo)
{
	if (this == &foo)
		return *this;

	study_id = foo.study_id;
	study_descr = foo.study_descr;
	study_date = foo.study_date;
	study_time = foo.study_time;
	patient_id = foo.patient_id;
	patient_name = foo.patient_name;
	patient_birthday = foo.patient_birthday;
	patient_sex = foo.patient_sex;
	study_instance_uid = foo.study_instance_uid;
	children = foo.children;

	return *this;
}

ConquestArchiveDialog::ConquestArchiveDialog( BaseObjectType* cobject,
	const Glib::RefPtr<Gtk::Builder>& builder)
	:
	Gtk::Dialog(cobject),
	builder_(builder),
	treeview_(0),
	entry_patient_name_(0),
	entry_patient_id_(0),
	entry_study_id_(0),
	checkbutton_case_sensitive_(0),
	label_found_(0),
	button_additional_query_(0),
	button_find_(0),
	button_clear_(0),
	conn_(0)
{
	init_ui();

	connect_signals();

	load_preferences();

	show_all_children();
}

ConquestArchiveDialog::~ConquestArchiveDialog()
{
/*
	if (conn_) {
		conn_->disconnect();
		delete conn_;
		conn_ = 0;
	}
	save_preferences();
*/
}

void
ConquestArchiveDialog::init_ui()
{
	treestore_ = Gtk::TreeStore::create(model_columns);
	builder_->get_widget( "treeview-archive-items", treeview_);
	treeview_->set_model(treestore_);
	treeview_->expand_all();

	Glib::RefPtr<Gtk::TreeSelection> select = treeview_->get_selection();
	select->set_mode(Gtk::SELECTION_SINGLE);

	builder_->get_widget( "entry-patient-name", entry_patient_name_);
	builder_->get_widget( "entry-patient-id", entry_patient_id_);
	builder_->get_widget( "entry-study-id", entry_study_id_);
	builder_->get_widget( "checkbutton-case-sensitive",
		checkbutton_case_sensitive_);
	builder_->get_widget( "label-found", label_found_);
	builder_->get_widget( "button-additional-query", button_additional_query_);
	builder_->get_widget( "button-find", button_find_);
	builder_->get_widget( "button-clear", button_clear_);

	create_columns();

	Glib::ustring dir = app.prefs.get<Glib::ustring>( "ConQuest",
		"localhost-server-directory");

	DICOM::ConquestFiles cfiles(dir.raw());
	DICOM::ConquestSettings csettings = cfiles.get_settings();
	Glib::ustring conninfo_ = csettings.get_pgsql_conninfo();
	connect_database(conninfo_.c_str());
}

void
ConquestArchiveDialog::create_columns()
{
	// Study ID column
	int cc = treeview_->append_column( Q_("Study|ID"), model_columns.study_id);
	Gtk::TreeViewColumn* column = treeview_->get_column(cc - 1);
	column->set_resizable();
	column->set_alignment(Gtk::ALIGN_CENTER);

	// Study Description column
	cc = treeview_->append_column( Q_("Study|Description"),
		model_columns.study_descr);
	column = treeview_->get_column(cc - 1);
	column->set_resizable();
	column->set_alignment(Gtk::ALIGN_CENTER);

	// Study Date column
	cc = treeview_->append_column( Q_("Study|Date"), model_columns.study_date);
	column = treeview_->get_column(cc - 1);
	column->set_resizable();
	column->set_alignment(Gtk::ALIGN_CENTER);

	Gtk::CellRenderer* renderer = column->get_first_cell_renderer();
	renderer->property_xalign() = 0.5;

	// Study Time column
	cc = treeview_->append_column( Q_("Study|Time"), model_columns.study_time);
	column = treeview_->get_column(cc - 1);
	column->set_resizable();
	column->set_alignment(Gtk::ALIGN_CENTER);

	renderer = column->get_first_cell_renderer();
	renderer->property_xalign() = 0.5;

	// Patient's ID column
	cc = treeview_->append_column( Q_("Patient|ID"), model_columns.patient_id);
	column = treeview_->get_column(cc - 1);
	column->set_resizable();
	column->set_alignment(Gtk::ALIGN_CENTER);

	// Patient's Name column
	cc = treeview_->append_column( Q_("Patient|Name"),
		model_columns.patient_name);
	column = treeview_->get_column(cc - 1);
	column->set_resizable();
	column->set_alignment(Gtk::ALIGN_CENTER);

	// Patient's Date of Birth column
	cc = treeview_->append_column( Q_("Patient|Date of Birth"),
		model_columns.patient_birthday);
	column = treeview_->get_column(cc - 1);
	column->set_resizable();
	column->set_alignment(Gtk::ALIGN_CENTER);

	renderer = column->get_first_cell_renderer();
	renderer->property_xalign() = 0.5;

	// Patient's Sex column
	cc = treeview_->append_column( Q_("Patient|Sex"),
		model_columns.patient_sex);
	column = treeview_->get_column(cc - 1);
	column->set_resizable();
	column->set_alignment(Gtk::ALIGN_CENTER);

	renderer = column->get_first_cell_renderer();
	renderer->property_xalign() = 0.5;
}

void
ConquestArchiveDialog::connect_signals()
{
	treeview_->signal_row_activated().connect(
		sigc::mem_fun( *this, &ConquestArchiveDialog::on_treeview_row_activated));
	button_find_->signal_clicked().connect(
		sigc::mem_fun( *this, &ConquestArchiveDialog::on_find_clicked));
	button_clear_->signal_clicked().connect(
		sigc::mem_fun( *this, &ConquestArchiveDialog::on_clear_clicked));
}

void
ConquestArchiveDialog::connect_database(const char* conninfo)
{
/*
	try {
		conn_ = new pqxx::connection(conninfo);

		conn_->prepare( prepared1, query1);
		conn_->prepare( prepared2, query2);
	}
	catch (const std::exception& err) {
		Gtk::MessageDialog dialog( *this, "", false, Gtk::MESSAGE_ERROR,
			Gtk::BUTTONS_CLOSE);

		dialog.set_title(_("Connecting Error"));
		dialog.set_message(_("Unable to establish a connection."));
		dialog.run();

		button_find_->set_sensitive(false);
		button_clear_->set_sensitive(false);

		delete conn_;
		conn_ = 0;
	}
*/
}

void
ConquestArchiveDialog::fill_dicom_query( Gtk::TreeModel::Row& row,
	DcmDataset& dataset)
{
	std::string str;
	Glib::ustring ustr;
	ustr = row[model_columns.patient_id];
	dataset.putAndInsertString( DCM_PatientID, ustr.c_str());

	str = row[model_columns.study_instance_uid];
	dataset.putAndInsertString( DCM_StudyInstanceUID, str.c_str());

	dataset.putAndInsertString( DCM_QueryRetrieveLevel, "PATIENT");
}

void
ConquestArchiveDialog::on_response(int)
{
	hide();
}

void
ConquestArchiveDialog::on_treeview_row_activated(
	const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn*)
{
	DcmDataset query_dataset;
	bool query_ok = false;
	Gtk::TreeModel::iterator iter = treestore_->get_iter(path);
	if (iter) {
		Gtk::TreeModel::Row row = *iter;
		if (row[model_columns.visible]) {

			ConfirmationDialog dialog(*this);
			int res = dialog.run();

			if (res == Gtk::RESPONSE_YES) {
				fill_dicom_query( row, query_dataset);
				query_ok = true;
			}
		}
	}

	if (query_ok) {
		MoveDicomDialog* dialog = MoveDicomDialog::create();
		if (dialog) {
			dialog->signal_move_result().connect(sigc::bind(
				sigc::mem_fun( *this, &ConquestArchiveDialog::on_move_result),
				dialog));
			dialog->move_query( query_dataset, _PATH_TMP);
			dialog->run();

			delete dialog;
		}
	}
}

void
ConquestArchiveDialog::on_move_result( bool result, const Glib::ustring& msg,
	MoveDicomDialog* dialog)
{
	if (result) {
		std::string filename = msg;
		signal_moved_filename_(filename);
	}
	else {
		ErrorDialog dialog( *this, msg);
		dialog.run();
	}
	dialog->response(0);
}

void
ConquestArchiveDialog::on_clear_clicked()
{
	treestore_->clear();
	archive_items_.clear();
	label_found_->set_text("");
}

void
ConquestArchiveDialog::on_find_clicked()
{
/*
	Glib::ustring pn = entry_patient_name_->get_text();
	Glib::ustring pi = entry_patient_id_->get_text();
	Glib::ustring si = entry_study_id_->get_text();

	pqxx::result r;

	treestore_->clear();
	archive_items_.clear();
	label_found_->set_text("");

	try {
		pqxx::work w(*conn_);

		r = checkbutton_case_sensitive_->get_active() ?
			w.prepared(prepared2)(pn.raw())(pi.raw())(si.raw()).exec() :
			w.prepared(prepared1)(pn.raw())(pi.raw())(si.raw()).exec();

		if (r.empty()) {
			throw std::runtime_error("Can't exec prepared statement!");
		}
		else {
			const char* files = ngettext( "file has been found",
				"files have been found", r.size());

			Glib::ustring txt = Glib::ustring::compose( "%1 %2",
				Glib::ustring::format(r.size()), Glib::ustring(files));

			label_found_->set_text(txt);
		}
	}
	catch (const std::exception&) {
		label_found_->set_text(_("Nothing has been found"));
		return;
	}

	pqxx::result::const_iterator iter = r.begin();

	std::vector<ArchiveItem> current_items;
	while (iter != r.end()) {
		pqxx::result::const_iterator iter_next =
			std::unique( iter, r.end(), compare_ids);

		for ( pqxx::result::const_iterator it = iter; it != iter_next; ++it) {
			ArchiveItem current_item;
			DICOM::ShortInfo info(it);

			current_item.study_id = info.study_id;
			current_item.study_descr = info.study_description;
			current_item.study_date = info.study_date;
			current_item.study_time = info.study_time_string();
			current_item.patient_id = info.patient_id;
			current_item.patient_name = info.patient_name;
			current_item.patient_birthday = info.patient_birthday;

			Glib::ustring sex = DICOM::format_person_sex(
				info.patient_sex, DICOM::SEX_STRING_SHORT);
 
			current_item.patient_sex = sex;
			current_item.study_instance_uid = info.study_instance_uid;

			current_items.push_back(current_item);
		}

		archive_items_.push_back(
			ArchiveItem( DICOM::ShortInfo::summary_info(iter), current_items));

		current_items.clear();

		iter = std::unique( iter, r.end(), compare_ids);
	};

	std::for_each( archive_items_.begin(), archive_items_.end(),
		sigc::mem_fun( *this, &ConquestArchiveDialog::add_archive_item));
*/ 
}

void
ConquestArchiveDialog::add_archive_item(const ArchiveItem& foo)
{
	Gtk::TreeRow row = *(treestore_->append());

	row[model_columns.study_id] = foo.study_id;
	row[model_columns.visible] = false;

	// Add Children:
	for ( std::vector<ArchiveItem>::const_iterator it = foo.children.begin();
		it != foo.children.end(); ++it) {

		const ArchiveItem& child = *it;

		Gtk::TreeRow child_row = *(treestore_->append(row.children()));

		child_row[model_columns.study_id] = child.study_id;
		child_row[model_columns.study_descr] = child.study_descr;
		child_row[model_columns.study_date] =
			child.study_date.format_string("%x");
		child_row[model_columns.study_time] = child.study_time;
		child_row[model_columns.patient_id] = child.patient_id;
		child_row[model_columns.patient_name] = child.patient_name;
		child_row[model_columns.patient_birthday] =
			child.patient_birthday.format_string("%x");
		child_row[model_columns.patient_sex] = child.patient_sex;
		child_row[model_columns.study_instance_uid] = child.study_instance_uid;
		child_row[model_columns.visible] = true;
	}
}

void
ConquestArchiveDialog::load_preferences()
{
	std::list<int> list = app.prefs.get< Glib::ArrayHandle<int> >( "Gui",
		"conquest-archive-dialog-columns-width");

	std::list<Gtk::TreeViewColumn*> columns = treeview_->get_columns();

	for ( ColumnsIter it = columns.begin(); it != columns.end(); ++it) {
		(*it)->set_sizing(Gtk::TREE_VIEW_COLUMN_FIXED);

		if (list.front()) {
			(*it)->set_fixed_width(list.front());
			(*it)->set_resizable(true);
		}

		list.pop_front();
	}

	int h = app.prefs.get<int>( "Gui", "conquest-archive-dialog-height");
	int w = app.prefs.get<int>( "Gui", "conquest-archive-dialog-width");

	resize( w, h);
}

void
ConquestArchiveDialog::save_preferences()
{
	int w, h;

	get_size( w, h);

	app.prefs.set( "Gui", "conquest-archive-dialog-height", h);
	app.prefs.set( "Gui", "conquest-archive-dialog-width", w);

	std::list<int> list;
	std::list<Gtk::TreeViewColumn*> columns = treeview_->get_columns();

	for ( ColumnsIter it = columns.begin(); it != columns.end(); ++it)
		list.push_back((*it)->get_width());

	app.prefs.set( "Gui", "conquest-archive-dialog-columns-width", list);
}

sigc::signal< void, const std::string&>
ConquestArchiveDialog::signal_moved_filename()
{
	return signal_moved_filename_;
}

ConquestArchiveDialog*
ConquestArchiveDialog::create()
{
	ConquestArchiveDialog* dialog = 0;
	std::string filename(builder_conquest_archive_dialog_filename);

	Glib::RefPtr<Gtk::Builder> builder = create_from_file(filename);

	if (builder)
		//Get the GtkBuilder-instantiated dialog
		builder->get_widget_derived( "dialog-window", dialog);

	return dialog;
}

} // namespace UI

} // namespace ScanAmati
