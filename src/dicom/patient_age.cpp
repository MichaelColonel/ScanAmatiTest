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

#include "patient_age.hpp"

namespace ScanAmati {

namespace DICOM {

Glib::ustring
PatientAge::age_string() const
{
	const char* suffix = 0;

	switch (type_) {
	case AGE_YEARS:
		suffix = ngettext( "year", "years", value_);
		break;
	case AGE_MONTHS:
		suffix = ngettext( "month", "months", value_);
		break;
	case AGE_WEEKS:
		suffix = ngettext( "week", "weeks", value_);
		break;
	case AGE_DAYS:
		suffix = ngettext( "day", "days", value_);
		break;
	default:
		break;
	}

	return (suffix) ? Glib::ustring::compose( "%1 %2",
		Glib::ustring::format(value_),
		Glib::ustring(suffix)): Glib::ustring(_("Unknown"));
}

} // namespace DICOM

} // namespace ScanAmati
