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

#include <string>
#include <vector>

namespace ScanAmati {

/**
 * Application directory.
 */
std::string get_rc_dir();
std::string get_rc_file();

std::string get_dicom_servers_file();

/** \brief Checks scanner id directory.
 * 
 * Check if scanner id directory exists and has all required files.
 * 
 * \param id Scanner id string.
 * \return true if successfull false otherwise. 
 */
bool check_scanner_id_files(const std::string&);

std::string get_lining_file(const std::string&);
std::string get_temperature_file(const std::string&);
std::string get_bad_strips_file(const std::string&);
std::string get_radiation_output_file(bool use_preset = false);

bool check_scanner_lining_file(const std::string& id);
bool check_scanner_temperature_file(const std::string& id);
bool check_scanner_bad_strips_file(const std::string& id);
bool check_radiation_output_file(bool use_preset = false);

std::vector<double>
equal_distant_points( double lower, double upper, unsigned int points);

namespace UI {

struct ActionState {
	const char* action_name;
	bool sensitivity; // active of disable
};

} // namespace UI

} // namespace ScanAmati
