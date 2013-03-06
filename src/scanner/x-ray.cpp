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

#include <cstddef>

#include "x-ray.hpp"

#define XRAY_DIG_360_VOLTAGE_MIN  40
#define XRAY_DIG_360_VOLTAGE_MAX 100

namespace {

// Portable X-ray unit DIG-360 made by DONGMUN CO., LTD
const struct DIG360Parameters {
	XrayVoltageRange voltage_range; // kV, value included
	double current; // mA
	XrayExposureRange exposure_range; // mAs
} dongmun_dig360_defaults[] = {
	{ XrayVoltageRange( 40, 60), 35.0, XrayExposureRange( 0.4, 100.0) },
	{ XrayVoltageRange( 61, 70), 30.0, XrayExposureRange( 0.4, 64.0) },
	{ XrayVoltageRange( 61, 70), 25.0, XrayExposureRange( 80.0, 100.0) },
	{ XrayVoltageRange( 71, 80), 30.0, XrayExposureRange( 0.4, 32.0) },
	{ XrayVoltageRange( 71, 80), 25.0, XrayExposureRange( 40.0, 80.0) },
	{ XrayVoltageRange( 81, 90), 25.0, XrayExposureRange( 0.4, 40.0) },
	{ XrayVoltageRange( 81, 90), 20.0, XrayExposureRange( 50.0, 80.0) },
	{ XrayVoltageRange( 91, 100), 20.0, XrayExposureRange( 0.4, 50.0) },
	{ XrayVoltageRange( 91, 100), 16.0, XrayExposureRange( 64.0, 80.0) },
	{ } // Terminating Entry
};

const struct DIG360Ranges {
	XrayVoltageRange voltage_range; // kV, value included
	XrayExposureRange exposure_range; // mAs
} dongmun_dig360_ranges[] = {
	{ XrayVoltageRange( 40, 70), XrayExposureRange( 0.4, 100.0) },
	{ XrayVoltageRange( 71, 100), XrayExposureRange( 0.4, 80.0) },
	{ } // Terminating Entry
};

const double dongmun_dig360_exposures[] = {
	0.4, 0.5, 0.6, 0.8, 1.0, 1.3, 1.6, 2.0, 2.5,
	3.2, 4.0, 5.0, 6.4, 8.0, 10., 13., 16., 20.,
	25., 32., 40., 50., 64., 80., 100.
};

} // namespace

namespace ScanAmati {

double
XrayParameters::current() const
{
	double current = -1.0;
	int i = 0;
	while (dongmun_dig360_defaults[i].current != 0.0) {
		XrayVoltageRange vrange = dongmun_dig360_defaults[i].voltage_range;
		XrayExposureRange erange = dongmun_dig360_defaults[i].exposure_range;

		bool v = (voltage >= vrange.first && voltage <= vrange.second);
		bool e = (exposure >= erange.first && exposure <= erange.second); 
		if (v && e) {
			current = dongmun_dig360_defaults[i].current;
			break;
		}
		++i;
	}
	return current;
}

XrayExposureRange
XrayParameters::exposure_range(unsigned int voltage)
{
	XrayExposureRange range( 0, 0);
	int i = 0;
	while (dongmun_dig360_ranges[i].voltage_range != XrayVoltageRange( 0, 0))
	{
		XrayVoltageRange vrange = dongmun_dig360_ranges[i].voltage_range;

		if (voltage >= vrange.first && voltage <= vrange.second) {
			range = dongmun_dig360_ranges[i].exposure_range;
			break;
		}
		++i;
	}
	return range;
}

std::vector<double>
XrayParameters::exposures()
{
	size_t size = sizeof(dongmun_dig360_exposures) / sizeof(double);

	return std::vector<double>( dongmun_dig360_exposures,
		dongmun_dig360_exposures + size);
}

double
XrayParameters::exposure_time() const
{
	double xray_current = this->current();
	return (exposure / xray_current);
}

XrayVoltageRange
XrayParameters::voltage_range()
{
	return XrayVoltageRange( XRAY_DIG_360_VOLTAGE_MIN,
		XRAY_DIG_360_VOLTAGE_MAX);
}

} // namespace ScanAmati
