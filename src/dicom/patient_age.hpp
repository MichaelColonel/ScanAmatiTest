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

#include <iomanip>
#include <glibmm/date.h>
#include <glibmm/i18n.h>
#include "typedefs.hpp"

namespace ScanAmati {

namespace DICOM {

class PatientAge {
public:
	PatientAge( AgeType type = AGE_YEARS, guint16 value = 0)
		: type_(type), value_(value) {}
	PatientAge(const Glib::Date& birth_date);
	PatientAge( const Glib::Date& birth_date, const Glib::Date& study_date);
	PatientAge(const Glib::ustring& dicom_age_string); // "025Y" for example
	~PatientAge() {}
	Glib::ustring age_string() const;
	Glib::ustring dicom_age_string() const;
	AgeType age_type() const { return type_; }
	guint16 age_value() const { return value_; }
	bool empty() const;
private:
	AgeType type_;
	guint16 value_ : 10; // [ 0, 999]
};

inline
PatientAge::PatientAge(const Glib::ustring& age_string)
	:
	type_(AGE_YEARS),
	value_(0)
{
	char str[4];
	age_string.copy( str, 3, 0);
	str[3] = '\0';
	value_ = atoi(str);

	switch (age_string[3]) {
	case 'D':
		type_ = AGE_DAYS;
		break;
	case 'W':
		type_ = AGE_WEEKS;
		break;
	case 'M':
		type_ = AGE_MONTHS;
		break;
	case 'Y':
		type_ = AGE_YEARS;
		break;
	default:
		type_ = AGE_YEARS;
		value_ = 0;
		break;
	}
}

inline
PatientAge::PatientAge(const Glib::Date& birth_date)
	:
	type_(AGE_YEARS),
	value_(0)
{
	Glib::Date current;
	current.set_time_current();

	int days = birth_date.days_between(current);
	if (days <= 0)
		return;

	if (days / 365) {
		type_ = AGE_YEARS;
		value_ = days / 365;
	}
	else if (days / 30) {
		type_ = AGE_MONTHS;
		value_ = days / 30;
	}
	else if (days / 7) {
		type_ = AGE_WEEKS;
		value_ = days / 7;
	}
	else {
		type_ = AGE_DAYS;
		value_ = days;
	}
}

inline
PatientAge::PatientAge( const Glib::Date& birth_date,
	const Glib::Date& study_date)
	:
	type_(AGE_DAYS),
	value_(0)
{
	int days = birth_date.days_between(study_date);
	if (days <= 0)
		return;

	if (days / 365) {
		type_ = AGE_YEARS;
		value_ = days / 365;
	}
	else if (days / 30) {
		type_ = AGE_MONTHS;
		value_ = days / 30;
	}
	else if (days / 7) {
		type_ = AGE_WEEKS;
		value_ = days / 7;
	}
	else {
		type_ = AGE_DAYS;
		value_ = days;
	}
}

inline
Glib::ustring
PatientAge::dicom_age_string() const
{
	const char* suffix = 0;
	switch (type_) {
	case AGE_YEARS:
		suffix = "Y";
		break;
	case AGE_MONTHS:
		suffix = "M";
		break;
	case AGE_WEEKS:
		suffix = "W";
		break;
	case AGE_DAYS:
		suffix = "D";
		break;
	default:
		break;
	}

	return (suffix) ? Glib::ustring::compose( "%1%2",
		Glib::ustring::format( std::setfill(L'0'), std::setw(3), value_),
		Glib::ustring(suffix)) : Glib::ustring("000Y");
}

inline
bool
PatientAge::empty() const
{
	return (type_ == AGE_YEARS && value_ == 0);
}

} // namespace DICOM

} // namespace ScanAmati
