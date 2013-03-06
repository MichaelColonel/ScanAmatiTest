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

#include "utils.hpp"

namespace {

const char* const patient_sex_long[] = {
	N_("Female"),
	N_("Male"),
	N_("Other")
};

const char* const patient_sex_short[] = {
	N_("F"),
	N_("M"),
	N_("O")
};

} // namespace

namespace ScanAmati {

namespace DICOM {

Glib::ustring
name_components_to_dicom( const Glib::ustring& last_name,
	const Glib::ustring& first_name, const Glib::ustring& middle_name,
	const Glib::ustring& prefix, const Glib::ustring& suffix)
{
	OFString first(first_name.c_str());
	OFString middle(middle_name.c_str());
	OFString last(last_name.c_str());
	OFString pref(prefix.c_str());
	OFString suff(suffix.c_str());

	OFString dicom_name;
	Glib::ustring name;

	if (DcmPersonName::getStringFromNameComponents( last, first, middle,
		pref, suff, dicom_name).good()) {
		const char* str = dicom_name.c_str();
		name = Glib::ustring(str);
	}
	return name;
}

bool
dicom_to_name_components( const Glib::ustring& dicom_name,
	Glib::ustring& last_name, Glib::ustring& first_name,
	Glib::ustring& middle_name, Glib::ustring& prefix,
	Glib::ustring& suffix)
{
	OFString first, middle, last, pref, suff;

	const char* str = dicom_name.c_str();
	OFString name(str);

	if (DcmPersonName::getNameComponentsFromString( name, last, first, middle,
		pref, suff).good()) {
		const char* str = last.c_str();
		last_name = Glib::ustring(str);
		str = first.c_str();
		first_name = Glib::ustring(str);
		str = middle.c_str();
		middle_name = Glib::ustring(str);
		str = pref.c_str();
		prefix = Glib::ustring(str);
		str = suff.c_str();
		suffix = Glib::ustring(str);
	}
	else
		return false;
	return true;
}

Glib::ustring
format_person_name(const Glib::ustring& dicom_name)
{
	OFString first, middle, last, prefix, suffix;

	const char* str = dicom_name.c_str();
	OFString name(str);

	Glib::ustring format_name;
	if (DcmPersonName::getNameComponentsFromString( name, last, first, middle,
		prefix, suffix).good()) {
		const char* str = last.c_str();
		Glib::ustring last_name(str);
		str = first.c_str();
		Glib::ustring first_name(str);
		str = middle.c_str();
		Glib::ustring middle_name(str);

		format_name = Glib::ustring::compose( "%1 %2 %3", last_name, first_name,
		middle_name);
		if (middle_name.empty())
			format_name = Glib::ustring::compose( "%1 %2", last_name,
				first_name);
		if (first_name.empty())
			format_name = Glib::ustring::compose( "%1", last_name);
	}

	return format_name;
}

Glib::ustring
format_person_sex( SexType sex, SexStringType type)
{
	Glib::ustring text;

	const char* const* strings;
	switch (type) {
	case SEX_STRING_SHORT:
		strings = patient_sex_short;
		break;
	case SEX_STRING_LONG:
	default:
		strings = patient_sex_long;
		break;
	}

	switch (sex) {
	case SEX_MALE:
		text = gettext(strings[1]);
		break;
	case SEX_FEMALE:
		text = gettext(strings[0]);
		break;
	case SEX_OTHER:
	default:
		text = gettext(strings[2]);
	}
	return text;
}

Glib::ustring
format_oftime(const OFTime& time)
{
	OFString ofstr;
	Glib::ustring str;
	if (time.getISOFormattedTime(ofstr))
		str = Glib::ustring(ofstr.c_str());
	return str;
}

unsigned long
Dataset::size( const E_TransferSyntax xfer, const E_EncodingType enctype)
{
	unsigned long size = 0;
	if (!elementList->empty()) {
		elementList->seek(ELP_first);
		do {
			DcmObject* object = elementList->get();
			size += object->calcElementLength( xfer, enctype);
		} while (elementList->seek(ELP_next));
	}
	return size;
}

} // namespace DICOM

} // namespace ScanAmati
