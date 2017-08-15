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

#include "builtin_chip_capacities.hpp"

namespace {

const struct Capacities {
	double capacity;
	const char* code;
	char value;
} builtin_capacities[] = {
	{ 1.0, "001", '1' }, // 1 pF
	{ 2.5, "010", '2' }, // 2.5 pF
	{ 3.5, "011", '3' }, // (1 + 2.5) pF
	{ 6.0, "100", '4' }, // 6 pF
	{ 7.0, "101", '5' }, // (6 + 1) pF
	{ 8.5, "110", '6' }, // (6 + 2.5) pF
	{ 9.5, "111", '7' }, // (6 + 2.5 + 1) pF
	{ } // Terminating Entry
};

} // namespace

namespace ScanAmati {

namespace Scanner {

std::vector<double>
BuiltinCapacities::capacities()
{
	std::vector<double> capacities;

	int i = 0;
	while (builtin_capacities[i].code) {
		capacities.push_back(builtin_capacities[i].capacity);
		++i;
	}

	return capacities;
}

const char*
BuiltinCapacities::capacity_code(double capacity)
{
	const char* code = 0;

	int i = 0;
	while (builtin_capacities[i].code) {
		if (builtin_capacities[i].capacity == capacity) {
			code = builtin_capacities[i].code;
			break;
		}
		++i;
	}
	return code;
}

char
BuiltinCapacities::capacity_char_code(double capacity)
{
	char value = '7';

	int i = 0;
	while (builtin_capacities[i].code) {
		if (builtin_capacities[i].capacity == capacity) {
			value = builtin_capacities[i].value;
			break;
		}
		++i;
	}
	return value;
}

} // namespace Scanner

} // namespace ScanAmati
