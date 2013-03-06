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

#include <glibmm/date.h>
#include <glibmm/i18n.h>
#include "typedefs.hpp"

namespace ScanAmati {

namespace DICOM {

inline
const char*
dicom_sex_string(SexType sex)
{
	const char* str = 0;
	switch (sex) {
	case SEX_FEMALE:
		str = "F";
		break;
	case SEX_MALE:
		str = "M";
		break;
	case SEX_OTHER:
	default:
		str = "O";
		break;
	}
	return str;
}

inline
SexType
sex_type(const Glib::ustring& str)
{
	SexType sex = SEX_OTHER;

	switch (str[0]) {
	case 'm':
	case 'M':
		sex = SEX_MALE;
		break;
	case 'f':
	case 'F':
		sex = SEX_FEMALE;
		break;
	default:
		sex = SEX_OTHER;
		break;
	}
	return sex;
}

inline
PositionStringMap
patient_positions()
{
	PositionStringMap map;

	map.insert(PositionStringPair( POSITION_SUPINE,
		Q_("Position|Supine")));
	map.insert(PositionStringPair( POSITION_PRONE,
		Q_("Position|Prone")));
	map.insert(PositionStringPair( POSITION_DECUBITUS_LEFT,
		Q_("Position|Decubitus Left")));
	map.insert(PositionStringPair( POSITION_DECUBITUS_RIGHT,
		Q_("Position|Decubitus Right")));

	return map;
}

inline
OrientationStringMap
patient_orientations()
{
	OrientationStringMap map;

	map.insert(OrientationStringPair(
		ORIENTATION_HEAD_FIRST, Q_("Orientation|Head First")));
	map.insert(OrientationStringPair(
		ORIENTATION_FEET_FIRST, Q_("Orientation|Feet First")));

	return map;
}

Glib::ustring
name_components_to_dicom( const Glib::ustring& first_name,
	const Glib::ustring& middle_name, const Glib::ustring& last_name,
	const Glib::ustring& prefix = "", const Glib::ustring& suffix = "");

bool
dicom_to_name_components( const Glib::ustring& dicom_name,
	Glib::ustring& last_name, Glib::ustring& first_name,
	Glib::ustring& middle_name, Glib::ustring& prefix,
	Glib::ustring& suffix);

Glib::ustring format_person_name(const Glib::ustring& dicom_name);
Glib::ustring format_person_sex( SexType sex, SexStringType type);

Glib::ustring format_oftime(const OFTime& time);

class Dataset : public DcmDataset {
public:
	unsigned long size(
		const E_TransferSyntax xfer = EXS_LittleEndianImplicit,
		const E_EncodingType enctype = EET_UndefinedLength);
};

} // namespace DICOM

} // namespace ScanAmati
