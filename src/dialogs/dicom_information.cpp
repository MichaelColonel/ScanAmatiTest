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

#include <glibmm/i18n.h>
#include <gtkmm/stock.h>
#include <gtkmm/entry.h>
#include <gtkmm/button.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/togglebutton.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/textview.h>
#include <gtkmm/combobox.h>
#include <gtkmm/liststore.h>
#include <gtkmm/calendar.h>

// files from src directory begin
#include "application.hpp"
#include "global_strings.hpp"
#include "dicom/summary_information.hpp"
// files from src directory begin

#include "utils.hpp"
#include "person_name.hpp"
#include "dicom_information.hpp"

namespace {

const struct PatientIdType {
	const char* label;
	const char* value;
} patient_id_types[] = {
	{ N_("Text"), "TEXT" },
	{ N_("RFID"), "RFID" },
	{ N_("Barcode"), "BARCODE" },
	{ 0 }
};

const struct ModelColumns : public Gtk::TreeModel::ColumnRecord {
	ModelColumns() { add(label); add(value); }
	Gtk::TreeModelColumn<Glib::ustring> label;
	Gtk::TreeModelColumn<std::string> value;
} model_columns;

Glib::RefPtr<Gtk::ListStore>
patient_id_type_liststore()
{
	Glib::RefPtr<Gtk::ListStore> liststore = Gtk::ListStore::create(model_columns);

	int i = 0;
	while (patient_id_types[i].value) {
		Gtk::TreeModel::Row row = *(liststore->append());
		row[model_columns.label] = gettext(patient_id_types[i].label);
		row[model_columns.value] = patient_id_types[i].value;
		++i;
	}
	return liststore;
}

}

namespace ScanAmati {

namespace UI {

DicomInformationDialog::DicomInformationDialog(
	BaseObjectType* cobject,
	const Glib::RefPtr<Gtk::Builder>& builder)
	:
	Gtk::Dialog(cobject),
	builder_(builder),
	entry_patient_id_(0),
	combobox_patient_id_type_(0),
	entry_patient_name_(0),
	button_patient_name_(0),
	entry_patient_birth_date_(0),
	button_patient_birth_date_(0),
	entry_patient_age_(0),
	togglebutton_patient_age_(0),
	radiobutton_patient_sex_female_(0),
	radiobutton_patient_sex_male_(0),
	radiobutton_patient_sex_other_(0),
	spinbutton_patient_weight_(0),
	spinbutton_patient_height_(0),
	entry_patient_other_ids_(0),
	entry_patient_other_names_(0),
	entry_patient_occupation_(0),
	entry_patient_address_(0),
	entry_patient_telephone_numbers_(0),
	entry_patient_ethnic_group_(0),
	entry_patient_religious_preference_(0),
	textview_patient_comments_(0),
	entry_study_id_(0),
	entry_study_description_(0),
	entry_study_accession_number_(0),
	entry_study_referring_physician_name_(0),
	entry_study_referring_physician_address_(0),
	entry_study_institution_name_(0),
	entry_study_institution_address_(0),
	entry_study_institution_department_(0),
	entry_series_number_(0),
	entry_series_description_(0),
	entry_series_protocol_name_(0),
	entry_series_performing_physicians_name_(0),
	button_series_performing_physicians_name_(0),
	entry_series_operators_name_(0),
	button_series_operators_name_(0)
{
	init_ui();

	connect_signals();

	show_all_children();
}

DicomInformationDialog::~DicomInformationDialog()
{
	int w, h;

	get_size( w, h);

	app.prefs.set( "Gui", "dicom-information-dialog-height", h);
	app.prefs.set( "Gui", "dicom-information-dialog-width", w);
}

void
DicomInformationDialog::init_ui()
{
	builder_->get_widget( "entry-patient-id", entry_patient_id_);
	builder_->get_widget( "combobox-patient-id-type", combobox_patient_id_type_);
	Glib::RefPtr<Gtk::ListStore> liststore = patient_id_type_liststore();
	combobox_patient_id_type_->set_model(liststore);
	combobox_patient_id_type_->set_active(0);

	builder_->get_widget( "entry-patient-name", entry_patient_name_);
	builder_->get_widget( "button-patient-name", button_patient_name_);
	builder_->get_widget( "entry-patient-birth-date", entry_patient_birth_date_);
	builder_->get_widget( "button-patient-birth-date", button_patient_birth_date_);
	builder_->get_widget( "entry-patient-age", entry_patient_age_);
	builder_->get_widget( "togglebutton-patient-age", togglebutton_patient_age_);
	builder_->get_widget( "radiobutton-patient-sex-female", radiobutton_patient_sex_female_);
	builder_->get_widget( "radiobutton-patient-sex-male", radiobutton_patient_sex_male_);
	builder_->get_widget( "radiobutton-patient-sex-other", radiobutton_patient_sex_other_);

	builder_->get_widget( "spinbutton-patient-weight", spinbutton_patient_weight_);
	builder_->get_widget( "spinbutton-patient-height", spinbutton_patient_height_);
	builder_->get_widget( "entry-patient-other-ids", entry_patient_other_ids_);
	builder_->get_widget( "entry-patient-other-names", entry_patient_other_names_);
	builder_->get_widget( "entry-patient-occupation", entry_patient_occupation_);
	builder_->get_widget( "entry-patient-address", entry_patient_address_);
	builder_->get_widget( "entry-patient-telephone-numbers", entry_patient_telephone_numbers_);
	builder_->get_widget( "entry-patient-ethnic-group", entry_patient_ethnic_group_);
	builder_->get_widget( "entry-patient-religious-preference", entry_patient_religious_preference_);
	builder_->get_widget( "textview-patient-comments", textview_patient_comments_);

	builder_->get_widget( "entry-study-id", entry_study_id_);
	builder_->get_widget( "entry-study-description", entry_study_description_);
	builder_->get_widget( "entry-study-accession-number", entry_study_accession_number_);
	builder_->get_widget( "entry-study-referring-physician-name", entry_study_referring_physician_name_);
	builder_->get_widget( "entry-study-referring-physician-address", entry_study_referring_physician_address_);
	builder_->get_widget( "entry-study-institution-name", entry_study_institution_name_);
	builder_->get_widget( "entry-study-institution-address", entry_study_institution_address_);
	builder_->get_widget( "entry-study-institution-department", entry_study_institution_department_);

	builder_->get_widget( "entry-series-number", entry_series_number_);
	builder_->get_widget( "entry-series-description", entry_series_description_);
	builder_->get_widget( "entry-series-protocol-name", entry_series_protocol_name_);
	builder_->get_widget( "entry-series-performing-physicians-name", entry_series_performing_physicians_name_);
	builder_->get_widget( "button-series-performing-physicians-name", button_series_performing_physicians_name_);
	builder_->get_widget( "entry-series-operators-name", entry_series_operators_name_);
	builder_->get_widget( "button-series-operators-name", button_series_operators_name_);

	int h = app.prefs.get<int>( "Gui", "dicom-information-dialog-height");
	int w = app.prefs.get<int>( "Gui", "dicom-information-dialog-width");
	resize( w, h);
}

void
DicomInformationDialog::connect_signals()
{
	button_patient_name_->signal_clicked().connect(sigc::mem_fun(
		*this, &DicomInformationDialog::on_patient_name_clicked));
	button_patient_birth_date_->signal_clicked().connect(sigc::mem_fun(
		*this, &DicomInformationDialog::on_patient_birth_date_clicked));
	togglebutton_patient_age_->signal_toggled().connect(sigc::mem_fun(
		*this, &DicomInformationDialog::on_patient_age_toggled));
}

void
DicomInformationDialog::on_response(int res)
{
	switch (res) {
	case 0: // Close
		break;
	case 1: // OK
		{
			DICOM::SummaryInfo info;
			get_patient_info(info);
			get_study_info(info);
			get_series_info(info);

			signal_new_dicom_info_(info);
		}
		break;
	default:
		break;
	}

	hide();
}

void
DicomInformationDialog::set_dicom_info(const DICOM::SummaryInfo& info)
{
	set_patient_info(info);
	set_study_info(info);
	set_series_info(info);
}

void
DicomInformationDialog::set_patient_info(const DICOM::SummaryInfo& summary)
{
	DICOM::SummaryInfo* info = const_cast<DICOM::SummaryInfo*>(&summary);
	DICOM::PatientInfo* patient = info;
	Glib::ustring text;

	entry_patient_name_->set_text(patient->get_name());
	entry_patient_id_->set_text(patient->get_id());
	set_patient_id_type("TEXT");

	Glib::Date date = patient->get_birth_date();
	entry_patient_birth_date_->set_text(date.format_string("%x"));

	togglebutton_patient_age_->set_active(false);
	DICOM::PatientAge age = info->get_patient_age();
	entry_patient_age_->set_text(age.age_string());

	set_patient_sex_type(info->get_patient_sex_type());
	spinbutton_patient_height_->set_value(patient->get_height());
	spinbutton_patient_weight_->set_value(patient->get_weight());
/*
	text = patient->get_LO(DCM_OtherPatientIDs);
	entry_patient_other_ids_->set_text(text);
*/
	text = patient->get_PN(DCM_OtherPatientNames);
	entry_patient_other_names_->set_text(text);

	text = patient->get_SH(DCM_Occupation);
	entry_patient_occupation_->set_text(text);

	text = patient->get_LO(DCM_PatientAddress);
	entry_patient_address_->set_text(text);

	text = patient->get_SH(DCM_PatientTelephoneNumbers);
	entry_patient_address_->set_text(text);

	text = patient->get_SH(DCM_EthnicGroup);
	entry_patient_ethnic_group_->set_text(text);

	text = patient->get_LO(DCM_PatientReligiousPreference);
	entry_patient_religious_preference_->set_text(text);

	text = patient->get_comments();
	Glib::RefPtr<Gtk::TextBuffer> buff = Gtk::TextBuffer::create();
	buff->set_text(text);
	textview_patient_comments_->set_buffer(buff);
}

void
DicomInformationDialog::get_patient_info(DICOM::SummaryInfo& summary)
{
	DICOM::PatientInfo* patient = summary.get_patient_info();
	Glib::ustring text;

	text = entry_patient_name_->get_text();
	patient->set_name(text);

	text = entry_patient_id_->get_text();
	patient->set_id(text);

	text = get_patient_id_type();
	patient->set_CS( DCM_TypeOfPatientID, text);

	text = entry_patient_birth_date_->get_text();
	Glib::Date date;
	date.set_parse(text);
	if (date.valid())
		patient->set_birth_date(date);

	if (togglebutton_patient_age_->get_active()) {
		text = entry_patient_age_->get_text();
		summary.set_patient_age(text);
	}

	DICOM::SexType sex = get_patient_sex_type();
	summary.set_patient_sex(sex);

	double weight = spinbutton_patient_weight_->get_value();
	patient->set_weight(weight);

	double height = spinbutton_patient_height_->get_value();
	patient->set_height(height);
/*
	text = entry_patient_other_ids_->get_text();
	patient->set_LO( DCM_OtherPatientIDs, text);
*/
	text = entry_patient_other_names_->get_text();
	patient->set_PN( DCM_OtherPatientNames, text);

	text = entry_patient_occupation_->get_text();
	patient->set_SH( DCM_Occupation, text);

	text = entry_patient_address_->get_text();
	patient->set_LO( DCM_PatientAddress, text);

	text = entry_patient_address_->get_text();
	patient->set_SH( DCM_PatientTelephoneNumbers, text);

	text = entry_patient_ethnic_group_->get_text();
	patient->set_SH( DCM_EthnicGroup, text);

	text = entry_patient_religious_preference_->get_text();
	patient->set_LO( DCM_PatientReligiousPreference, text);

	Glib::RefPtr<const Gtk::TextBuffer> buff = textview_patient_comments_->get_buffer();
	patient->set_comments(buff->get_text());
}

void
DicomInformationDialog::set_study_info(const DICOM::SummaryInfo& summary)
{
	DICOM::SummaryInfo* info = const_cast<DICOM::SummaryInfo*>(&summary);
	DICOM::StudyInfo* study = info;
	Glib::ustring text;

	entry_study_id_->set_text(study->get_id());
	entry_study_description_->set_text(study->get_description());

	text = study->get_SH(DCM_AccessionNumber);
	entry_study_accession_number_->set_text(text);

	entry_study_referring_physician_name_->set_text(study->get_physician_name());

	text = study->get_ST(DCM_ReferringPhysicianAddress);
	entry_study_referring_physician_address_->set_text(text);

	entry_study_institution_name_->set_text(study->get_institution_name());
	entry_study_institution_address_->set_text(study->get_institution_address());

	text = study->get_LO(DCM_InstitutionalDepartmentName);
	entry_study_institution_department_->set_text(text);
}

void
DicomInformationDialog::get_study_info(DICOM::SummaryInfo& summary)
{
	DICOM::StudyInfo* study = summary.get_study_info();
	Glib::ustring text;

	text = entry_study_id_->get_text();
	study->set_id(text);

	text = entry_study_description_->get_text();
	study->set_description(text);

	text = entry_study_accession_number_->get_text();
	study->set_SH( DCM_AccessionNumber, text);

	text = entry_study_referring_physician_name_->get_text();
	study->set_physician_name(text);

	text = entry_study_referring_physician_address_->get_text();
	study->set_ST( DCM_ReferringPhysicianAddress, text);

	text = entry_study_institution_name_->get_text();
	study->set_institution_name(text);

	text = entry_study_institution_address_->get_text();
	study->set_institution_address(text);

	text = entry_study_institution_department_->get_text();
	study->set_LO( DCM_InstitutionalDepartmentName, text);
}

void
DicomInformationDialog::set_series_info(const DICOM::SummaryInfo& summary)
{
	DICOM::SummaryInfo* info = const_cast<DICOM::SummaryInfo*>(&summary);
	DICOM::StudyInfo* study = info;
	Glib::ustring text;

	int number = study->get_IS(DCM_SeriesNumber);
	entry_series_number_->set_text(Glib::ustring::format(number));

	text = study->get_LO(DCM_SeriesDescription);
	entry_series_description_->set_text(text);

	entry_series_protocol_name_->set_text(study->get_protocol_name());

	text = study->get_PN(DCM_PerformingPhysicianName);
	entry_series_performing_physicians_name_->set_text(text);

	entry_series_operators_name_->set_text(study->get_operator_name());
}

void
DicomInformationDialog::get_series_info(DICOM::SummaryInfo& summary)
{
	DICOM::StudyInfo* study = summary.get_study_info();
	Glib::ustring text;

	text = entry_series_number_->get_text();
	study->set_IS( DCM_SeriesNumber, atoi(text.c_str()));

	text = entry_series_description_->get_text();
	study->set_LO( DCM_SeriesDescription, text);

	text = entry_series_protocol_name_->get_text();
	study->set_protocol_name(text);

	text = entry_series_performing_physicians_name_->get_text();
	study->set_PN( DCM_PerformingPhysicianName, text);

	text = entry_series_operators_name_->get_text();
	study->set_operator_name(text);
}

void
DicomInformationDialog::on_patient_birth_date_clicked()
{
	Gtk::Calendar calendar;
	Gtk::Dialog dialog( _("Patient's Date of Birth"), *this, true);
	Gtk::VBox* vbox = dialog.get_vbox();
	vbox->pack_start(calendar);
	dialog.add_button( Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	dialog.add_button( Gtk::Stock::OK, Gtk::RESPONSE_OK);

	calendar.show();
	int result = dialog.run();

	switch (result) {
	case Gtk::RESPONSE_CANCEL:
		break;
	case Gtk::RESPONSE_OK:
		{
			Glib::Date date;
			calendar.get_date(date);
			if (date.valid())
				entry_patient_birth_date_->set_text(date.format_string("%x"));
		}
		break;
	default:
		break;
	}
}

void
DicomInformationDialog::on_patient_age_toggled()
{
	bool state = togglebutton_patient_age_->get_active();
	entry_patient_age_->set_sensitive(state);
}

std::string
DicomInformationDialog::get_patient_id_type()
{
	std::string value;

	Gtk::TreeIter iter = combobox_patient_id_type_->get_active();
	if (iter) {
		Gtk::TreeRow row = *iter;
		value = row[model_columns.value];
	}
	return value;
}

bool
DicomInformationDialog::foreach_patient_id_type(
	const Gtk::TreeModel::iterator& iter, const std::string& id)
{
	bool res = false;
	if (iter) {
		Gtk::TreeRow row = *iter;
		const std::string& value = row[model_columns.value];
		if (value == id) {
			combobox_patient_id_type_->set_active(iter);
			res = true;
		}
	}
	return res;
}

void
DicomInformationDialog::set_patient_id_type(const std::string& id_type)
{
	Gtk::TreeModel::SlotForeachIter slot = sigc::bind(
		sigc::mem_fun( *this, &DicomInformationDialog::foreach_patient_id_type),
		id_type);

	Glib::RefPtr<Gtk::TreeModel> model = combobox_patient_id_type_->get_model();
	model->foreach_iter(slot);
}

DICOM::SexType
DicomInformationDialog::get_patient_sex_type()
{
	DICOM::SexType sex;
	if (radiobutton_patient_sex_female_->get_active())
		sex = DICOM::SEX_FEMALE;
	else if (radiobutton_patient_sex_male_->get_active())
		sex = DICOM::SEX_MALE;
	else
		sex = DICOM::SEX_OTHER;
	return sex;
}

void
DicomInformationDialog::set_patient_sex_type(DICOM::SexType sex)
{
	switch (sex) {
	case DICOM::SEX_FEMALE:
		radiobutton_patient_sex_female_->set_active();
		break;
	case DICOM::SEX_MALE:
		radiobutton_patient_sex_male_->set_active();
		break;
	case DICOM::SEX_OTHER:
	default:
		radiobutton_patient_sex_other_->set_active();
		break;
	}
}

void
DicomInformationDialog::on_patient_name_clicked()
{
	Glib::ustring name = entry_patient_name_->get_text();

	PersonNameDialog* dialog = PersonNameDialog::create(name);
	if (dialog) {
		dialog->signal_name_changed().connect(sigc::mem_fun(
			*entry_patient_name_, &Gtk::Entry::set_text));
		dialog->run();
		delete dialog;
	}
}

sigc::signal< void, const DICOM::SummaryInfo&>
DicomInformationDialog::signal_new_dicom_info()
{
	return signal_new_dicom_info_;
}

DicomInformationDialog*
DicomInformationDialog::create()
{
	DicomInformationDialog* dialog = 0;
	std::string filename(builder_dicom_information_dialog_filename);

	Glib::RefPtr<Gtk::Builder> builder = create_from_file(filename);

	if (builder)
		//Get the GtkBuilder-instantiated dialog
		builder->get_widget_derived( "dialog-window", dialog);

	return dialog;
}

} // namespace UI

} // namespace ScanAmati
