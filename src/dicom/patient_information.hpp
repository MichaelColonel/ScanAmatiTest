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

#include "information.hpp"

namespace ScanAmati {

namespace DICOM {

class PatientInfo : virtual public Info {
public:
	virtual ~PatientInfo() {}
	Glib::Date get_birth_date() { return Info::get_date(DCM_PatientBirthDate); }
	bool set_birth_date(const Glib::Date& date);
	double get_height() { return Info::get_DS(DCM_PatientSize); }
	bool set_height(double h) { return Info::set_DS( DCM_PatientSize, h); }
	double get_weight() { return Info::get_DS(DCM_PatientWeight); }
	bool set_weight(double w) { return Info::set_DS( DCM_PatientWeight, w); }
	Glib::ustring get_name() { return Info::get_PN(DCM_PatientName); }
	bool set_name(const Glib::ustring& name) { return Info::set_PN( DCM_PatientName, name); }
	Glib::ustring& sex() { return Info::value(DCM_PatientSex); }
	Glib::ustring get_id() { return Info::value(DCM_PatientID); }
	bool set_id(const Glib::ustring& id) { return Info::set_LO( DCM_PatientID, id); }
	Glib::ustring get_age() { return Info::get_AS(DCM_PatientAge); }
	bool set_age(const Glib::ustring& age) { return Info::set_AS( DCM_PatientAge, age); } 
	Glib::ustring get_address() { return Info::get_LO(DCM_PatientAddress); }
	bool set_address(const Glib::ustring& address) { return Info::set_LO( DCM_PatientAddress, address); }
	Glib::ustring get_comments() { return Info::get_LT(DCM_PatientComments); }
	bool set_comments(const Glib::ustring& comments) { return Info::set_LT( DCM_PatientComments, comments); }
	Glib::ustring get_telephone_numbers();
	bool set_telephone_numbers(const Glib::ustring& numbers) { return Info::set_SH( DCM_PatientTelephoneNumbers, numbers); }
protected:
	PatientInfo(DcmDataset* dataset) : Info(dataset) { this->load(dataset); }
	virtual bool load(DcmDataset* dataset);
};

inline
Glib::ustring
PatientInfo::get_telephone_numbers()
{
	return Info::get_SH(DCM_PatientTelephoneNumbers);
}

inline
bool
PatientInfo::set_birth_date(const Glib::Date& date)
{
	if (Info::set_date( DCM_PatientBirthDate, date)) {
		Glib::ustring age = PatientAge(date).dicom_age_string();
		return Info::set_AS( DCM_PatientAge, age);
	}
	return false;
}

} // namespace DICOM

} // namespace ScanAmati
