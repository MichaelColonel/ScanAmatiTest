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

#include <algorithm>
#include <numeric>

/* files from src directory begin */
#include "ccmath_wrapper.h"
#include "utils.hpp"
/* files from src directory end */

#include "assemble.hpp"

namespace {

bool
absolute_less( gint16 v1, gint16 v2)
{
	return abs(v1) < abs(v2);
}

const double tension = 200.;

} // namespace

namespace ScanAmati {
	
namespace Scanner {

void
Assemble::clear_after_disconnect()
{
	pedestals.clear();
	raw_data.reset();
	bad_strips.clear();
	code_counts_map.clear();
}

void
Assemble::calculate_lining(gint16 count)
{
	calculate_lining( code_counts_map, count);
}

void
Assemble::calculate_lining( const CodeCountsMap& map, gint16 count)
{
	std::vector<guint8> coefficients(map.size());
	std::vector<gint16> column(map.size());

	unsigned int j;
	CodeCountsMap::const_iterator iter;

	for ( int i = 0; i < SCANNER_STRIPS_PER_CHIP_REAL; ++i) {
		for ( j = 0, iter = map.begin(); iter != map.end(); ++iter, ++j) {
			column[j] = iter->second[i];
			coefficients[j] = iter->first;
		}

		std::transform( column.begin(), column.end(), column.begin(),
			std::bind2nd( std::minus<gint16>(), count));

		int pos = std::min_element( column.begin(),
			column.end(), absolute_less) - column.begin();

		lining[i] = coefficients[pos];
	}
}

CodeCountsMap
Assemble::expand_code_counts_map() const
{
	int n = code_counts_map.size() - 1;

	double* x = new double[SCANNER_LINING_CODES];
	double* y = new double[SCANNER_LINING_CODES];
	double* p = new double[SCANNER_LINING_CODES];

	CodeCountsMap new_map;

	// initiate new code counts map
	int i = SCANNER_LINING_CODE_MIN;
	while (i <= SCANNER_LINING_CODE_MAX) {
		new_map.insert(CodeCountsPair( guint8(i),
			Image::DataVector(SCANNER_STRIPS_PER_CHIP_REAL)));
		++i;
	}

	for ( i = 0; i < SCANNER_STRIPS_PER_CHIP_REAL; ++i) {
		unsigned int k = 0;
		CodeCountsMap::const_iterator iter;
		for ( iter = code_counts_map.begin(); iter != code_counts_map.end();
			++iter, ++k) {
			x[k] = double(iter->first);
			y[k] = double(iter->second[i]);
		}
		ccm_cspl( x, y, p, n, tension);
		for ( int j = SCANNER_LINING_CODE_MIN; j <= SCANNER_LINING_CODE_MAX;
			++j) {
			new_map[j][i] = ccm_splfit( double(j), x, y, p, n, tension);
		}
	}
	delete [] x;
	delete [] y;
	delete [] p;

	return new_map;
}

} // namespace Scanner

} // namespace ScanAmati
