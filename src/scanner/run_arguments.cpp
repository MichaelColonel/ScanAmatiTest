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
/* files from src directory end */

#include "defines.hpp"

#include "run_arguments.hpp"

namespace ScanAmati {

namespace Scanner {

RunArguments::RunArguments()
	:
	with_acquisition(true),
	with_exposure(true),
	movement_type(MOVEMENT_BOTH),
	acquisition(),
	chip_codes( array_chip_codes, array_chip_codes + SCANNER_CHIPS)
{
}

RunArguments::RunArguments(Command* com)
	:
	with_acquisition(true),
	with_exposure(true),
	movement_type(MOVEMENT_BOTH),
	acquisition(),
	chip_codes( array_chip_codes, array_chip_codes + SCANNER_CHIPS)
{
	commands.push_back(CommandSharedPtr(com));
}

RunArguments::RunArguments(const std::vector<Command*>& coms)
	:
	with_acquisition(true),
	with_exposure(true),
	movement_type(MOVEMENT_BOTH),
	acquisition(),
	chip_codes( array_chip_codes, array_chip_codes + SCANNER_CHIPS)
{
	for ( std::vector<Command*>::const_iterator it = coms.begin();
		it != coms.end(); ++it) {
		commands.push_back(CommandSharedPtr(*it));
	}
}

RunArguments::~RunArguments()
{
}

} // namespace Scanner

} // namespace ScanAmati
