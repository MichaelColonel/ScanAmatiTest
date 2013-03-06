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

#include <glibmm/miscutils.h>
#include <glibmm/fileutils.h>

#include <glib.h>

#include "global_strings.hpp"
#include "utils.hpp"

#include <cassert>

namespace {

std::string
get_scanner_id_dir(const std::string& id)
{
  return (ScanAmati::data_dir + id);
}

} // namespace

namespace ScanAmati {

std::string
get_rc_dir()
{
  return (Glib::get_user_config_dir() + G_DIR_SEPARATOR_S + rc_dir);
}

std::string
get_rc_file()
{
  return (get_rc_dir() + G_DIR_SEPARATOR_S + rc_file);
}

std::string
get_dicom_servers_file()
{
  return (get_rc_dir() + G_DIR_SEPARATOR_S + dicom_servers_file);
}

std::string
get_lining_file(const std::string& id)
{
  return (get_rc_dir() + G_DIR_SEPARATOR_S + id + "." + lining_file_extension);
}

std::string
get_radiation_output_file(bool use_preset)
{
	std::string path = (use_preset) ? std::string(data_dir) : get_rc_dir();
	return (path + G_DIR_SEPARATOR_S + radiation_output_file);
}

std::string
get_temperature_file(const std::string& id)
{
  return (std::string(data_dir) + G_DIR_SEPARATOR_S + id + "."
  	+ temperature_file_extension);
}

std::string
get_bad_strips_file(const std::string& id)
{
  return (std::string(data_dir) + G_DIR_SEPARATOR_S + id + "."
  	+ bad_strips_file_extension);
}

bool
check_scanner_lining_file(const std::string& id)
{
	bool res;
	std::string rc_dir = get_rc_dir();
	std::string line_file = get_lining_file(id);

	res = Glib::file_test( rc_dir, Glib::FILE_TEST_EXISTS);
	if (!res)
		return false;

	res = Glib::file_test( line_file, Glib::FILE_TEST_IS_REGULAR);
	if (!res)
		return false;

	return true;
}

bool
check_scanner_temperature_file(const std::string& id)
{
	std::string filename = get_temperature_file(id);

	return Glib::file_test( filename, Glib::FILE_TEST_IS_REGULAR);
}

bool
check_scanner_bad_strips_file(const std::string& id)
{
	std::string filename = get_bad_strips_file(id);

	return Glib::file_test( filename, Glib::FILE_TEST_IS_REGULAR);
}

bool
check_radiation_output_file(bool use_preset)
{
	std::string filename = get_radiation_output_file(use_preset);

	return Glib::file_test( filename, Glib::FILE_TEST_IS_REGULAR);
}

std::vector<double>
equal_distant_points( double lower, double upper, unsigned int points)
{
	assert(points > 1);
	assert(upper > lower);

	std::vector<double> data(points + 1);

	double step = (upper - lower) / points;
	double value = lower;
	for ( unsigned int i = 0; i < points + 1; ++i, value += step)
		data[i] = value;

	return data;
}

} // namespace ScanAmati
