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

/* files from src directory begin */
#include "global_strings.hpp"
#include "application.hpp"
/* files from src directory end */

#include "state.hpp"

namespace ScanAmati {

namespace Scanner {

void
State::save(const std::string& id) const
{
	app.prefs.set( id, conf_key_state_peltier_code, int(peltier_code_));
	app.prefs.set( id, conf_key_state_chip_capacity, capacity_);
	app.prefs.set( id, conf_key_state_temperature_control, temperature_control_);
	app.prefs.set( id, conf_key_state_temperature_average, temperature_average_);
	app.prefs.set( id, conf_key_state_temperature_spread, temperature_spread_);
}

bool
State::load(const std::string& id)
{
	std::string group = app.prefs.has_group(id) ? id : std::string(id_default);

	id_ = id;
	peltier_code_ = app.prefs.get<int>( group, conf_key_state_peltier_code);
	capacity_ = app.prefs.get<double>( group, conf_key_state_chip_capacity);
	temperature_control_ = app.prefs.get<bool>( group,
		conf_key_state_temperature_control);
	temperature_average_ = app.prefs.get<double>( group,
		conf_key_state_temperature_average);
	temperature_spread_ = app.prefs.get<double>( group,
		conf_key_state_temperature_spread);

	return app.prefs.has_group(id);
}

void
State::what_todo( Glib::ustring& what, Glib::ustring& todo) const
{
	what = what_;
	todo = todo_;
}

} // namespace Scanner

} // namespace ScanAmati
