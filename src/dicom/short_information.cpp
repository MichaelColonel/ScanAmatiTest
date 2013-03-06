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

#include <glibmm/convert.h>
#include "utils.hpp"
#include "short_information.hpp"

namespace ScanAmati {

namespace DICOM {

ShortInfo::ShortInfo(const pqxx::result::const_iterator& iter)
{
	Glib::ustring dicom_name = iter[3].as(std::string());
	patient_name = format_person_name(dicom_name);

	patient_id = Glib::locale_to_utf8(iter[0].as(std::string()));

	patient_sex = sex_type(Glib::ustring(iter[4].as(std::string())));

	Glib::Date date;
	date.set_parse(iter[5].as(std::string()));

	patient_birthday = date.valid() ? date :
		Glib::Date( 1, Glib::Date::JANUARY, 1900);

	study_id = Glib::locale_to_utf8(iter[1].as(std::string()));
	study_description = Glib::locale_to_utf8(iter[6].as(std::string()));

	date.set_parse(iter[2].as(std::string()));
	study_date = date.valid() ? date :
		Glib::Date( 1, Glib::Date::JANUARY, 1900);

	OFString time_string(iter[7].as(std::string()).c_str());
	study_time.setISOFormattedTime(time_string);

	study_instance_uid = iter[8].as(std::string());
}

Glib::ustring
ShortInfo::patient_summary() const
{
	return Glib::ustring::compose( _("ID - %1, %2 (%3 year of birth)"),
		patient_id,
		format_person_sex( patient_sex, SEX_STRING_SHORT),
		Glib::ustring::format(patient_birthday.get_year()));
}

Glib::ustring
ShortInfo::summary_info(const pqxx::result::const_iterator& iter)
{
	Glib::ustring id = Glib::locale_to_utf8(iter[0].as(std::string()));
	SexType sex = sex_type(Glib::ustring(iter[4].as(std::string())));

	Glib::Date gdate, date;
	gdate.set_parse(iter[5].as(std::string()));
	date = gdate.valid() ? gdate : Glib::Date( 1, Glib::Date::JANUARY, 1900);

	return Glib::ustring::compose( _("ID - %1, %2 (%3 year of birth)"),
		id,
		format_person_sex( sex, SEX_STRING_SHORT),
		Glib::ustring::format(date.get_year()));
}

Glib::ustring
ShortInfo::study_time_string() const
{
	OFString time_string;
	study_time.getISOFormattedTime(time_string);

	return Glib::ustring(time_string.c_str());
}

} // namespace DICOM

} // namespace ScanAmati
