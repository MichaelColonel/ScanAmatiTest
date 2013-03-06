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

#include <pqxx/result>
#include <vector>

#include "typedefs.hpp"

namespace ScanAmati {

namespace DICOM {

struct ShortInfo {
	ShortInfo();
	ShortInfo(const pqxx::result::const_iterator& iter);
	ShortInfo( const Glib::ustring&, const std::vector<ShortInfo>&);
	Glib::ustring study_time_string() const;
	Glib::ustring patient_summary() const;
	static Glib::ustring summary_info(const pqxx::result::const_iterator& iter);

	Glib::ustring study_id;
	Glib::ustring study_description;
	Glib::Date study_date;
	OFTime study_time;
	Glib::ustring patient_id;
	Glib::ustring patient_name;
	Glib::Date patient_birthday;
	SexType patient_sex;
	std::string study_instance_uid;
	std::vector<ShortInfo> children;
};

inline
ShortInfo::ShortInfo()
	:
	study_date( 1, Glib::Date::JANUARY, 1900),
	patient_birthday( 1, Glib::Date::JANUARY, 1900)
{
}

inline
ShortInfo::ShortInfo( const Glib::ustring& id,
	const std::vector<ShortInfo>& children_)
	:
	study_id(id),
	study_date( 1, Glib::Date::JANUARY, 1900),
	patient_birthday( 1, Glib::Date::JANUARY, 1900),
	children(children_)
{
}

} // namespace DICOM

} // namespace ScanAmati
