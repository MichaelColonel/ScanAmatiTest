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
#include "tag_string_map.hpp"

namespace ScanAmati {

namespace DICOM {

class Info : public TagStringMap {
public:
	virtual ~Info() {}
	Glib::Date get_date(const DcmTagKey&); // Glib Date
	bool set_date( const DcmTagKey&, const Glib::Date&);
	Glib::ustring get_PN(const DcmTagKey&); // Person Name
	bool set_PN( const DcmTagKey&, const Glib::ustring&);
	Glib::ustring get_CS(const DcmTagKey&); // Code String
	bool set_CS( const DcmTagKey&, const Glib::ustring&);
	Glib::ustring get_LO(const DcmTagKey&); // Long String
	bool set_LO( const DcmTagKey&, const Glib::ustring&);
	Glib::ustring get_LT(const DcmTagKey&); // Long Text
	bool set_LT( const DcmTagKey&, const Glib::ustring&);
	Glib::ustring get_ST(const DcmTagKey&); // Short Text
	bool set_ST( const DcmTagKey&, const Glib::ustring&);
	Glib::ustring get_SH(const DcmTagKey&); // Short String
	bool set_SH( const DcmTagKey&, const Glib::ustring&);
	Glib::ustring get_AS(const DcmTagKey&); // Age String
	bool set_AS( const DcmTagKey&, const Glib::ustring&);
	double get_DS(const DcmTagKey&); // Decimal (double)
	bool set_DS( const DcmTagKey&, double); 
	int get_IS(const DcmTagKey&); // Integer (int)
	bool set_IS( const DcmTagKey&, int);
protected:
	Info(DcmDataset* dataset = 0) : TagStringMap(dataset) {}
	OFTime get_TM(const DcmTagKey&); // Time
	bool set_TM( const DcmTagKey&, const OFTime&);
private:
	OFDate get_DA(const DcmTagKey&); // Date
	bool set_DA( const DcmTagKey&, const OFDate&);
};

inline
OFTime
Info::get_TM(const DcmTagKey& key)
{
	OFTime value;
	DcmTime* name = get_VR<DcmTime>(key);
	if (name) {
		name->getOFTime(value);
		delete name;
	}

	return value;
}

inline
bool
Info::set_TM( const DcmTagKey& key, const OFTime& value)
{
	DcmTime* name = set_VR<DcmTime>(key);
	if (name) {
		if (!name->setOFTime(value).good()) {
			delete name;
			return false;
		}
	}
	else
		return false;

	bool res = verify<DcmTime>( name, key);

	delete name;

	return res;
}

inline
OFDate
Info::get_DA(const DcmTagKey& key)
{
	OFDate value;
	DcmDate* name = get_VR<DcmDate>(key);
	if (name) {
		name->getOFDate(value);
		delete name;
	}

	return value;
}

inline
bool
Info::set_DA( const DcmTagKey& key, const OFDate& value) {
	DcmDate* name = set_VR<DcmDate>(key);
	if (name) {
		if (!name->setOFDate(value).good()) {
			delete name;
			return false;
		}
	}
	else
		return false;

	bool res = verify<DcmDate>( name, key);

	delete name;

	return res;
}

inline
Glib::Date
Info::get_date(const DcmTagKey& key)
{
	OFDate ofdate = get_DA(key);
	Glib::Date::Month month = static_cast<Glib::Date::Month>(ofdate.getMonth());
	Glib::Date gdate( ofdate.getDay(), month, ofdate.getYear());
	return gdate;
}

inline
bool
Info::set_date( const DcmTagKey& key, const Glib::Date& date)
{
	OFDate ofdate;
	ofdate.setDate( date.get_year(), date.get_month(), date.get_day());
	return set_DA( key, ofdate);
}

inline
Glib::ustring
Info::get_PN(const DcmTagKey& key)
{
	return get_string<DcmPersonName>(key);
}

inline
bool
Info::set_PN( const DcmTagKey& key, const Glib::ustring& string)
{
	return set_string<DcmPersonName>( key, string);
}

inline
Glib::ustring
Info::get_AS(const DcmTagKey& key)
{
	return get_string<DcmAgeString>(key);
}

inline
bool
Info::set_AS( const DcmTagKey& key, const Glib::ustring& string)
{
	return set_string<DcmAgeString>( key, string);
}

inline
Glib::ustring
Info::get_CS(const DcmTagKey& key)
{
	return get_string<DcmCodeString>(key);
}

inline
bool
Info::set_CS( const DcmTagKey& key, const Glib::ustring& string)
{
	return set_string<DcmCodeString>( key, string);
}

inline
Glib::ustring
Info::get_LO(const DcmTagKey& key)
{
	return get_string<DcmLongString>(key);
}

inline
bool
Info::set_LO( const DcmTagKey& key, const Glib::ustring& string)
{
	return set_string<DcmLongString>( key, string);
}

inline
Glib::ustring
Info::get_LT(const DcmTagKey& key)
{
	return get_string<DcmLongText>(key);
}

inline
bool
Info::set_LT( const DcmTagKey& key, const Glib::ustring& string)
{
	return set_string<DcmLongText>( key, string);
}

inline
Glib::ustring
Info::get_ST(const DcmTagKey& key)
{
	return get_string<DcmShortText>(key);
}

inline
bool
Info::set_ST( const DcmTagKey& key, const Glib::ustring& string)
{
	return set_string<DcmShortText>( key, string);
}

inline
Glib::ustring
Info::get_SH(const DcmTagKey& key)
{
	return get_string<DcmShortString>(key);
}

inline
bool
Info::set_SH( const DcmTagKey& key, const Glib::ustring& string)
{
	return set_string<DcmShortString>( key, string);
}

inline
double
Info::get_DS(const DcmTagKey& key)
{
	Float64 value = -1.;
	DcmDecimalString* name = get_VR<DcmDecimalString>(key);
	if (name) {
		name->getFloat64(value);
		delete name;
	}

	return value;
}

inline
bool
Info::set_DS( const DcmTagKey& key, double value)
{
	Glib::ustring string = Glib::ustring::format(value);
	return set_string<DcmDecimalString>( key, string);
}

inline
int
Info::get_IS(const DcmTagKey& key)
{
	Sint32 value = -1;
	DcmIntegerString* name = get_VR<DcmIntegerString>(key);
	if (name) {
		name->getSint32(value);
		delete name;
	}

	return value;
}

inline
bool
Info::set_IS( const DcmTagKey& key, int value)
{
	Glib::ustring string = Glib::ustring::format(value);
	return set_string<DcmIntegerString>( key, string);
}

} // namespace DICOM

} // namespace ScanAmati

