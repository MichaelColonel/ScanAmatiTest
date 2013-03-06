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

#include <gtkmm/dialog.h>
#include <gtkmm/builder.h>

// files from src directory begin
#include "dicom/typedefs.hpp"
// files from src directory end

namespace Gtk {
class Entry;
class Button;
class SpinButton;
class ToggleButton;
class RadioButton;
class ComboBox;
class TextView;
} // namespace Gtk

namespace ScanAmati {

namespace DICOM {
class PatientInfo;
class StudyInfo;
class SummaryInfo;
class ToggleButton;
} // namespace DICOM

namespace UI {

class DicomInformationDialog : public Gtk::Dialog {
public:
	static DicomInformationDialog* create();
	DicomInformationDialog( BaseObjectType* cobject,
		const Glib::RefPtr<Gtk::Builder>& builder);
	virtual ~DicomInformationDialog();

	void set_dicom_info(const DICOM::SummaryInfo&);
	sigc::signal< void, const DICOM::SummaryInfo&> signal_new_dicom_info();

protected:

	// UI methods:
	void init_ui();
	void connect_signals();
	virtual void on_response(int);

	void set_patient_info(const DICOM::SummaryInfo&);
	void set_study_info(const DICOM::SummaryInfo&);
	void set_series_info(const DICOM::SummaryInfo&);

	void get_patient_info(DICOM::SummaryInfo&);
	void get_study_info(DICOM::SummaryInfo&);
	void get_series_info(DICOM::SummaryInfo&);

	// Signal handlers:
	void on_patient_name_clicked();
	void on_patient_birth_date_clicked();
	void on_patient_age_toggled();

	std::string get_patient_id_type();
	void set_patient_id_type(const std::string& id_type);
	bool foreach_patient_id_type( const Gtk::TreeModel::iterator&,
		const std::string&);

	DICOM::SexType get_patient_sex_type();
	void set_patient_sex_type(DICOM::SexType);

	// UI Widgets:
	Glib::RefPtr<Gtk::Builder> builder_;
	sigc::signal< void, const DICOM::SummaryInfo&> signal_new_dicom_info_;

	Gtk::Entry* entry_patient_id_;
	Gtk::ComboBox* combobox_patient_id_type_;
	Gtk::Entry* entry_patient_name_;
	Gtk::Button* button_patient_name_;
	Gtk::Entry* entry_patient_birth_date_;
	Gtk::Button* button_patient_birth_date_;
	Gtk::Entry* entry_patient_age_;
	Gtk::ToggleButton* togglebutton_patient_age_;
	Gtk::RadioButton* radiobutton_patient_sex_female_;
	Gtk::RadioButton* radiobutton_patient_sex_male_;
	Gtk::RadioButton* radiobutton_patient_sex_other_;
	Gtk::SpinButton* spinbutton_patient_weight_;
	Gtk::SpinButton* spinbutton_patient_height_;
	Gtk::Entry* entry_patient_other_ids_;
	Gtk::Entry* entry_patient_other_names_;
	Gtk::Entry* entry_patient_occupation_;
	Gtk::Entry* entry_patient_address_;
	Gtk::Entry* entry_patient_telephone_numbers_;
	Gtk::Entry* entry_patient_ethnic_group_;
	Gtk::Entry* entry_patient_religious_preference_;
	Gtk::TextView* textview_patient_comments_;
	Gtk::Entry* entry_study_id_;
	Gtk::Entry* entry_study_description_;
	Gtk::Entry* entry_study_accession_number_;
	Gtk::Entry* entry_study_referring_physician_name_;
	Gtk::Entry* entry_study_referring_physician_address_;
	Gtk::Entry* entry_study_institution_name_;
	Gtk::Entry* entry_study_institution_address_;
	Gtk::Entry* entry_study_institution_department_;
	Gtk::Entry* entry_series_number_;
	Gtk::Entry* entry_series_description_;
	Gtk::Entry* entry_series_protocol_name_;
	Gtk::Entry* entry_series_performing_physicians_name_;
	Gtk::Button* button_series_performing_physicians_name_;
	Gtk::Entry* entry_series_operators_name_;
	Gtk::Button* button_series_operators_name_;
};

} // namespace UI

} // namespace ScanAmati
