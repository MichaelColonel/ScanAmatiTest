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

#include <vector>
#include <tr1/memory>

#include "acquisition.hpp"
#include "commands.hpp"

namespace ScanAmati {

namespace Scanner {

struct RunArguments {
	RunArguments();
	~RunArguments();
	RunArguments(unsigned int focal_distance);
	RunArguments(Command* com);
	RunArguments(const std::vector<Command*>& coms);
	bool with_acquisition;
	bool with_exposure;
	MovementType movement_type;
	Acquisition acquisition;
	std::vector<char> chip_codes;
	std::vector<CommandSharedPtr> commands;
};

} // namespace Scanner

} // namespace ScanAmati
