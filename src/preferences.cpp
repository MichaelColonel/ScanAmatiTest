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

#include <glibmm/i18n.h>
#include <glibmm/ustring.h>
#include <glibmm/fileutils.h>
#include <glibmm/convert.h>
#include <glibmm/iochannel.h>

#include "utils.hpp"
#include "preferences.hpp"

namespace {

const char defaults[] = 
"[ConQuest]\n"
"localhost-server-directory=/var/www/cgi-bin\n"
"[Gui]\n"
"window-height=600\n"
"window-width=800\n"
"window-position-x=0\n"
"window-position-y=0\n"
"window-hpaned-position=400\n"
"window-vpaned-position=500\n"
"image-acquisition-dialog-height=800\n"
"image-acquisition-dialog-width=600\n"
"image-acquisition-dialog-xray-high-voltage=40\n"
"image-acquisition-dialog-xray-exposure-pos=1\n"
"image-acquisition-dialog-memory-size=33554432\n"
"chip-capacities-dialog-height=200\n"
"chip-capacities-dialog-width=300\n"
"store-dicom-dialog-width=500\n"
"store-dicom-dialog-height=500\n"
"move-dicom-dialog-width=500\n"
"move-dicom-dialog-height=500\n"
"lining-acquisition-dialog-height=95\n"
"lining-acquisition-dialog-width=343\n"
"lining-acquisistion-dialog-accuracy=255\n"
"lining-adjustment-dialog-height=700\n"
"lining-adjustment-dialog-width=200\n"
"scanner-debug-dialog-height=400\n"
"scanner-debug-dialog-width=400\n"
"temperature-margins-dialog-height=95\n"
"temperature-margins-dialog-width=343\n"
"image-levels-dialog-height=300\n"
"image-levels-dialog-width=400\n"
"conquest-archive-dialog-height=644\n"
"conquest-archive-dialog-width=952\n"
"conquest-archive-dialog-columns-width=322;219;80;64;99;315;120;320;\n"
"dicom-informaion-dialog-height=500\n"
"dicom-informaion-dialog-width=500\n"
"[APRMXXX]\n"
"peltier-code=50\n"
"chip-capacity=6.0\n"
"temperature-control=true\n"
"temperature-average=7.0\n"
"temperature-spread=0.2\n";

}

namespace ScanAmati {

Preferences::Preferences()
{
}

Preferences::~Preferences()
{
}

bool
Preferences::load()
{
	if (!Glib::file_test( get_rc_file(), Glib::FILE_TEST_EXISTS)) {
		std::cout << "rc file doesn't exist" << std::endl;
		std::cout << "program will use default settings" << std::endl;
		return false;
	}
	else {
		try {
			keyfile_.load_from_file( get_rc_file(),
				Glib::KEY_FILE_KEEP_TRANSLATIONS |
				Glib::KEY_FILE_KEEP_COMMENTS);
		}
		catch (const Glib::KeyFileError& err) {
			switch (err.code()) {
			case Glib::KeyFileError::UNKNOWN_ENCODING:
				std::cerr << "unknown encoding of rc file" << std::endl;
				break;
			case Glib::KeyFileError::NOT_FOUND:
				std::cerr << "rc file not found" << std::endl;
				break;
			default:
				std::cerr << "error while loading rc file" << std::endl;
				break;
			}
			return false;
		}
	}

	return true;
}

void
Preferences::load_defaults()
{
	try {
		keyfile_.load_from_data( defaults,
			Glib::KEY_FILE_KEEP_TRANSLATIONS |
			Glib::KEY_FILE_KEEP_COMMENTS);
	}
	catch (const Glib::KeyFileError& err) {
		switch (err.code()) {
		case Glib::KeyFileError::UNKNOWN_ENCODING:
			std::cerr << "unknown encoding of the defaults data" << std::endl;
			break;
		default:
			break;
		}
	}
}

bool
Preferences::save()
{
	try {
		Glib::RefPtr<Glib::IOChannel> outfile =
			Glib::IOChannel::create_from_file( get_rc_file(), "w");
		outfile->write(keyfile_.to_data());
		outfile->flush();
	}
	catch (const Glib::Error& error) {
		std::cerr << _("Failed to save configuration file.") << std::endl;
		std::cerr << _("An error occurred while attempting to save the file ")
			+ get_rc_file() + _(", this file stores your preferences.");
		std::cerr << std::endl;
		std::cerr << _("The system reported this error as: ") + error.what();
		std::cerr << std::endl;
		return false;
	}
	return true;
}

bool
Preferences::has_group(const Glib::ustring& group) const
{
	return keyfile_.has_group(group);
}

Glib::ustring
Preferences::get( const Glib::ustring& group_name,
	const Glib::ustring& key, const Glib::ustring*) const
{
	return keyfile_.get_string( group_name, key);
}

int Preferences::get( const Glib::ustring& group_name,
	const Glib::ustring& key, const int*) const
{
	return keyfile_.get_integer( group_name, key);
}

bool Preferences::get( const Glib::ustring& group_name,
	const Glib::ustring& key, const bool*) const
{
	return keyfile_.get_boolean( group_name, key);
}

double Preferences::get( const Glib::ustring& group_name,
	const Glib::ustring& key, const double*) const
{
	return keyfile_.get_double( group_name, key);
}

Glib::ArrayHandle<int>
Preferences::get( const Glib::ustring& group_name,
	const Glib::ustring& key, const Glib::ArrayHandle<int>*) const
{
	return keyfile_.get_integer_list( group_name, key);
}

void
Preferences::set( const Glib::ustring& group_name, const Glib::ustring& key,
	const Glib::ustring& value)
{
	keyfile_.set_string( group_name, key, value);
}

void
Preferences::set( const Glib::ustring& group_name,
	const Glib::ustring& key, int value)
{
	keyfile_.set_integer( group_name, key, value);
}

void
Preferences::set( const Glib::ustring& group_name,
	const Glib::ustring& key, bool value)
{
	keyfile_.set_boolean( group_name , key, value);
}

void
Preferences::set( const Glib::ustring& group_name,
	const Glib::ustring& key, double value)
{
	keyfile_.set_double( group_name , key, value);
}

void
Preferences::set( const Glib::ustring& group_name,
	const Glib::ustring& key, Glib::ArrayHandle<int> value)
{
	keyfile_.set_integer_list( group_name, key, value);
}

} // namespace ScanAmati
