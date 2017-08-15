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

#include <numeric>
#include <algorithm>
#include <functional>
#include <cassert>
#include <cmath>

/* files from src directory begin */
#include "ccmath_wrapper.h"
#include "utils.hpp"
/* files from src directory end */

#include "data.hpp"
#include "calibration.hpp"

namespace {

bool
absolute_less( double v1, double v2)
{
	return fabs(v1) < fabs(v2);
}

} // namespace

namespace ScanAmati {

namespace Image {

double
Calibration::add_row( const DataVector& row, bool skip_bad_strips)
{
	double value = (skip_bad_strips) ? mean_skip(row) :
		std::accumulate( row.begin(), row.end(), 0.) / row.size();

/*
	switch (skip_bad_strips) {
	case false:
		value = std::accumulate( row.begin(), row.end(), 0.) / row.size();
		break;
	case true:
	default:
		value = mean_skip(row);
		break;
	}
*/
	add_row( value, row);

	return value;
}

void
Calibration::add_row( double value, const DataVector& row)
{
	map_.insert(CalibrationPair( value, row));
}

void
Calibration::add_row( double value, const DataSharedPtr& data,
	unsigned int lower, unsigned int upper)
{
	map_.insert(CalibrationPair( value, data->mean_row( lower, upper)));
}

double
Calibration::mean_skip(const DataVector& row)
{
	size_t size = row.size();
	std::vector<unsigned int> all(size); // all strips

	for ( size_t i = 0; i < size; ++i)
		all[i] = i + 1;

	std::vector<unsigned int>::const_iterator end = all.end();
	for ( std::vector<unsigned int>::const_iterator iter = skip_.begin();
		iter != skip_.end(); ++iter) {
		end = std::remove( all.begin(), all.end(), *iter);
		all.resize(end - all.begin());
	}

	double result = 0.;
	for ( std::vector<unsigned int>::const_iterator iter = all.begin();
		iter != all.end(); ++iter) {
		result += row[*iter];
	}
	return (result / all.size());
}

void
Calibration::expand( double min, double max)
{
	int n = map_.size() - 1;
	double* x = new double[map_.size()];
	double* y = new double[map_.size()];
	double* p = new double[map_.size()];

	size_t width = map_.begin()->second.size();

	std::vector<double> points = equal_distant_points( min, max, range_);

	CalibrationMap cmap = map_;

	for ( std::vector<double>::const_iterator it = points.begin();
		it != points.end(); ++it)
		map_.insert(CalibrationPair( *it, DataVector(width)));

	for ( size_t i = 0; i < width; ++i) {
		size_t j = 0;
		for ( CalibrationMap::const_iterator iter = cmap.begin();
			iter != cmap.end(); ++iter, ++j) {
			x[j] = iter->first;
			y[j] = iter->second[i];
		}
		ccm_cspl( x, y, p, n, tension_);

		for ( std::vector<double>::const_iterator it = points.begin();
			it != points.end(); ++it)
			map_[*it][i] = ccm_splfit( *it, x, y, p, n, tension_);
	}

	delete [] x;
	delete [] y;
	delete [] p;
}

DataSharedPtr
Calibration::calibrate(const DataSharedPtr& image)
{
	std::vector<double> factor(map_.size());
	std::vector<double> column(map_.size());
	std::vector<double> column_tmp(map_.size());

	DataSharedPtr result = Data::create( image->width(), image->height());

	std::vector<guint> all_strips(image->width());

	for ( guint i = 0; i < image->width(); ++i)
		all_strips[i] = i + 1;

	std::vector<guint>::const_iterator end = all_strips.end();
	for ( std::vector<guint>::const_iterator iter = skip_.begin();
		iter != skip_.end(); ++iter) {
		end = std::remove( all_strips.begin(), all_strips.end(), *iter);
		all_strips.resize(end - all_strips.begin());
	}

	for ( std::vector<guint>::const_iterator it = all_strips.begin();
		it != all_strips.end(); ++it) {
		unsigned int i = *it - 1;
		CalibrationMap::const_iterator iter;
		unsigned int j;
		for ( j = 0, iter = map_.begin(); iter != map_.end(); ++iter, ++j) {
			if (iter->second[i]) {
				column[j] = iter->second[i];
				factor[j] = iter->first / iter->second[i];
			}
			else {
				column[j] = 1;
				factor[j] = iter->first;
			}
		}

		for ( unsigned int k = 0; k < image->height(); ++k) {
			std::copy( column.begin(), column.end(), column_tmp.begin());

			gint16 pixel = image->pixel( i, k);

			std::transform( column_tmp.begin(), column_tmp.end(),
				column_tmp.begin(), std::bind2nd( std::minus<double>(), pixel));

			gsize pos = std::min_element( column_tmp.begin(),
				column_tmp.end(), absolute_less) - column_tmp.begin();

			result->pixel( i, k) = factor[pos] * pixel;
		}
	}

	return result;
}

} // namespace Image

} // namespace ScanAmati
