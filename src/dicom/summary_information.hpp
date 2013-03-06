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

#include "patient_age.hpp"
#include "patient_information.hpp"
#include "study_information.hpp"

#include "utils.hpp"

#define PrivateCreatorTag DcmTag( 0x0029, 0x0010, EVR_LO)
#define PrivateEffectiveDoseTag DcmTag( 0x0029, 0x1000, EVR_DS)
#define PrivateBodyPartTag DcmTag( 0x0029, 0x1010, EVR_LO)

namespace ScanAmati {

struct XrayParameters;

namespace Scanner {
class State;
} // namespace Scanner

namespace DICOM {

class SummaryInfo : public PatientInfo, public StudyInfo {
public:
	SummaryInfo(DcmDataset* dataset = 0);
	virtual ~SummaryInfo() {}
	PatientInfo* get_patient_info() { return dynamic_cast<PatientInfo*>(this); }
	StudyInfo* get_study_info() { return dynamic_cast<StudyInfo*>(this); }
	void save(DcmDataset* set);
	PatientAge get_patient_age();
	void set_patient_age(const PatientAge&);

	Glib::ustring get_patient_sex(SexStringType type = SEX_STRING_LONG);
	void set_patient_sex(SexType sex_type);
	SexType get_patient_sex_type();
	Glib::ustring get_diagnoses_description(); 
	bool set_diagnoses_description(const Glib::ustring&); 
	Glib::ustring get_series_description(); 
	bool set_series_description(const Glib::ustring&); 
	Glib::ustring get_image_comments(); 
	bool set_image_comments(const Glib::ustring&);
	bool set_position_orientation(const OrientationPositionPair&);
	bool get_position_orientation(OrientationPositionPair&);
	void set_xray_parameters(const XrayParameters&);
	void set_scanner_state(const Scanner::State&);
	void set_acquisition_current_date_time();
	Glib::ustring get_tooltip_text();
	Glib::ustring get_label_text();

private:
	void save_private_elements(DcmDataset* set);
};

inline
Glib::ustring
SummaryInfo::get_series_description()
{
	return StudyInfo::get_LO(DCM_SeriesDescription);
}

inline
bool
SummaryInfo::set_series_description(const Glib::ustring& descr)
{
	return StudyInfo::set_LO( DCM_SeriesDescription, descr);
}

inline
Glib::ustring
SummaryInfo::get_diagnoses_description()
{
	return StudyInfo::get_LO(DCM_AdmittingDiagnosesDescription);
}

inline
bool
SummaryInfo::set_diagnoses_description(const Glib::ustring& descr)
{
	return StudyInfo::set_LO( DCM_AdmittingDiagnosesDescription, descr);
}

inline
Glib::ustring
SummaryInfo::get_image_comments()
{
	return StudyInfo::get_LT(DCM_ImageComments);
}

inline
bool
SummaryInfo::set_image_comments(const Glib::ustring& comments)
{
	return StudyInfo::set_LT( DCM_ImageComments, comments);
}

inline
PatientAge
SummaryInfo::get_patient_age()
{
	Glib::ustring str = PatientInfo::get_age();
	PatientAge age(str);
	return age;
}

inline
void
SummaryInfo::set_patient_age(const PatientAge& age)
{
	PatientInfo::set_age(age.dicom_age_string());
}

inline
void
SummaryInfo::set_patient_sex(SexType sex)
{
	const char* str = dicom_sex_string(sex);
	PatientInfo::sex() = Glib::ustring(str);
}

inline
SexType
SummaryInfo::get_patient_sex_type()
{
	Glib::ustring sex = PatientInfo::sex();
	SexType type = sex_type(sex);
	return type;
}

} // namespace DICOM

} // namespace ScanAmati
