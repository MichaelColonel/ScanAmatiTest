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
#include <utility>

typedef std::pair< unsigned int, unsigned int> XrayVoltageRange;
typedef std::pair< double, double> XrayExposureRange;

namespace ScanAmati {

struct XrayParameters {
	XrayParameters( unsigned int volts, double exp)
		: voltage(volts), exposure(exp) {}
	static std::vector<double> exposures();
	static XrayVoltageRange voltage_range();
	static XrayExposureRange exposure_range(unsigned int voltage);
	double current() const;
	double exposure_time() const;
	unsigned int voltage;
	double exposure;
};

} // namespace ScanAmati
