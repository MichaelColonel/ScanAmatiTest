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
#include <glibmm/ustring.h>
#include "manager_state.hpp"
#include "builtin_chip_capacities.hpp"
#include "temperature_regulator.hpp"

namespace ScanAmati {

namespace Scanner {

class State {
friend class Manager;

public:
	State();
	ManagerState manager_state() const { return manager_state_; }
	std::string id() const { return id_; }
	char chip() const { return chip_; }
	double capacity() const { return capacity_; }
	double temperature() const { return temperature_; }
	void temperature_margins( double& average, double& spread) const;
	guint8 peltier_code() const { return peltier_code_; }
	bool temperature_control() const { return temperature_control_; }
	void what_todo( Glib::ustring& what, Glib::ustring& todo) const;

private:
	void save(const std::string& id) const;
	bool load(const std::string& id);

	ManagerState manager_state_;
	std::string id_;
	char chip_; // current chip
	double capacity_; // current chip capacity
	double temperature_; // current temperature
	double temperature_average_;
	double temperature_spread_;
	guint8 peltier_code_;
	bool temperature_control_;
	Glib::ustring what_;
	Glib::ustring todo_;
};

inline
State::State()
	:
	manager_state_(),
	id_("APRMXXX"),
	chip_(0),
	capacity_(SCANNER_DEFAULT_CHIP_CAPACITY),
	temperature_(0.),
	temperature_average_(SCANNER_DEFAULT_TEMPERATURE_AVERAGE),
	temperature_spread_(SCANNER_DEFAULT_TEMPERATURE_SPREAD),
	peltier_code_(UCHAR_MAX),
	temperature_control_(true)
{
}

inline
void
State::temperature_margins( double& average, double& spread) const
{
	average = temperature_average_;
	spread = temperature_spread_;
}
	
} // namespace Scanner

} // namespace ScanAmati
