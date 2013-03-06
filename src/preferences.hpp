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

#include <glibmm/keyfile.h>

#include <iostream>

namespace ScanAmati {

class Preferences {

public:
	Preferences();
	virtual ~Preferences();
	bool save();
	bool load();
	void load_defaults();

	template <typename T>
	T get( const Glib::ustring& group_name, const Glib::ustring& key) const {
		try {
			const T* nullpointer = 0;
			T value = get( group_name, key, nullpointer);
			return value;
		}
		catch (const Glib::KeyFileError& error) {
			std::cerr << "Group: " << group_name << ", Key: " << key
				<< " not found," << std::endl;
		}
	}

	void set( const Glib::ustring& group_name, const Glib::ustring& key,
		const Glib::ustring& value);
	void set( const Glib::ustring& group_name, const Glib::ustring& key,
		int value);
	void set( const Glib::ustring& group_name, const Glib::ustring& key,
		bool value);
	void set( const Glib::ustring& group_name, const Glib::ustring& key,
		double value);
	void set( const Glib::ustring& group_name, const Glib::ustring& key,
		Glib::ArrayHandle<int> values);
	bool has_group(const Glib::ustring& group) const;

protected:
	Glib::ustring get( const Glib::ustring& group_name,
		const Glib::ustring& key, const Glib::ustring*) const;
	int get( const Glib::ustring& group_name, const Glib::ustring& key,
		const int*) const;
	double get( const Glib::ustring& group_name, const Glib::ustring& key,
		const double*) const;
	bool get( const Glib::ustring& group_name, const Glib::ustring& key,
		const bool*) const;
	Glib::ArrayHandle<int> get( const Glib::ustring& group_name,
		const Glib::ustring& key, const Glib::ArrayHandle<int>*) const;

	Glib::KeyFile keyfile_;
};

} // namespace ScanAmati
