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

#include "typedefs.hpp"
#include <iostream>

struct DefaultTagString;

namespace ScanAmati {

namespace DICOM {

class TagStringMap {
public:
	virtual ~TagStringMap() {}
	DcmTagStringMap get_map() const { return map_; }
	bool save(DcmDataset* dataset);

protected:
	TagStringMap(DcmDataset* dataset = 0);
	virtual bool load( DcmDataset* dataset, const DefaultTagString*);
	Glib::ustring& value(const DcmTagKey& key) { return map_[key]; }
	Glib::ustring value(const DcmTagKey& key) const { return map_.at(key); }
	template<typename T> Glib::ustring get_string(const DcmTagKey&);
	template<typename T> bool set_string( const DcmTagKey&,
		const Glib::ustring&);
	template<typename T> T* get_VR(const DcmTagKey&);
	template<typename T> T* set_VR(const DcmTagKey&);
	template<typename T> bool verify( T*, const DcmTagKey&);
private:
	DcmTagStringMap map_;
};

template<typename T>
T*
TagStringMap::get_VR(const DcmTagKey& key)
{
	DcmElement* element = new T(DcmTag(key));

	if (element->putString(map_[key].c_str()).good()) {
		T* value = dynamic_cast<T*>(element);
		if (value)
			return value;
	}
	delete element;
	return 0;
}

template<typename T>
T*
TagStringMap::set_VR(const DcmTagKey& key)
{
	DcmElement* element = new T(DcmTag(key));

	T* value = dynamic_cast<T*>(element);
	if (value)
		return value;
	else
		delete element;

	return 0;
}

template<typename T>
bool
TagStringMap::verify( T* name, const DcmTagKey& key)
{
	if (name->verify(OFTrue).good()) {
		char* str = 0;
		if (name->getString(str).good()) {
			if (str) {
				map_[key] = Glib::ustring(str);
				return true;
			}
		}
	}
	return false;
}

template<typename T>
Glib::ustring
TagStringMap::get_string(const DcmTagKey& key)
{
	Glib::ustring string;
	T* name = get_VR<T>(key);
	if (name) {
		char* str = 0;
		if (name->getString(str).good())
			string = (str) ? Glib::ustring(str) : Glib::ustring();
		delete name;
	}

	return string;
}

template<typename T>
bool
TagStringMap::set_string( const DcmTagKey& key, const Glib::ustring& str)
{
	T* name = set_VR<T>(key);
	if (name) {
		if (!name->putString(str.c_str()).good()) {
			delete name;
			return false;
		}
	}
	else
		return false;

	bool res = false;
	if (name->verify(OFTrue).good()) {
		map_[key] = str;
		res = true;
	}

	delete name;

	return res;
}

inline
bool
TagStringMap::save(DcmDataset* set)
{
	bool i = true;
	DcmTagStringMap::const_iterator it;
	for ( it = map_.begin(); it != map_.end(); ++it) {
		if (it->first.getGroup() & 1) // Ignore private elements
			continue;
		if (set->putAndInsertString( it->first, it->second.c_str()).bad())
			i = false;
	}
	return i;
}

} // namespace DICOM

} // namespace ScanAmati

