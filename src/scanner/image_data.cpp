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

#include <cassert>
#include <algorithm>
#include <functional>

#include <Magick++/Image.h>
#include <Magick++/Exception.h>

#include "defines.hpp"

/* files from src directory begin */
#include "ccmath_wrapper.h"
/* files from src directory end */


#include "image_data.hpp"

namespace {

gint16
normalize_value(gint16 value)
{
	if (value <= SCANNER_ADC_COUNT_MIN)
		return 0;
	else if (value >= SCANNER_ADC_COUNT_MAX)
		return SCANNER_ADC_COUNT_MAX;
	else
		return value;
}

const double tension = 0.;

} // namespace

namespace ScanAmati {

namespace Image {

DataSharedPtr
Data::create_from_data( unsigned int width, unsigned int height,
	const gint16* data)
{
	return DataSharedPtr(new Data( width, height, data));
}

DataSharedPtr
Data::create( unsigned int width, unsigned int height)
{
	return DataSharedPtr(new Data( width, height));
}

DataSharedPtr
Data::create_from_shared(const DataSharedPtr& image)
{
	DataSharedPtr res;
	if (image && !image->empty())
		res = create_from_data( image->width(), image->height(), image->data());

	return res;
}

Data::Data( unsigned int width, unsigned int height)
	:
	width_(width),
	height_(height),
	data_(0)
{
	assert(width_ * height_);
	data_ = new gint16[width_ * height_];
}

Data::Data( unsigned int width, unsigned int height,
	const gint16* data)
	:
	width_(width),
	height_(height),
	data_(0)
{
	assert(width_ * height_);
	assert(data);

	data_ = new gint16[width_ * height_];
	std::copy( data, data + width * height, data_);
}

Data::Data( unsigned int width, unsigned int height,
	const DataVector& data)
	:
	width_(width),
	height_(height),
	data_(0)
{
	assert(width_ * height_);
	assert((width_ * height_) == data.size());

	data_ = new gint16[width_ * height_];
	std::copy( data.begin(), data.end(), data_);
}

Data::~Data()
{
	clear();
}

Data::Data(const Data& obj)
	:
	width_(obj.width_),
	height_(obj.height_),
	data_(0)
{
	if (!obj.empty()) {
		data_ = new gint16[width_ * height_];
		std::copy( obj.data_, obj.data_ + obj.width_ * obj.height_, data_);
	}
}

Data&
Data::operator=(const Data& obj)
{
	if (this == &obj)
		return *this;

	this->clear();

	if (!obj.empty()) {
		this->width_ = obj.width_;
		this->height_ = obj.height_;

		data_ = new gint16[width_ * height_];

		std::copy( obj.data_, obj.data_ + obj.width_ * obj.height_, data_);
	}
	
	return *this;
}

bool
Data::shift_columns( ColumnsShiftDirectionType dir,
	unsigned int times)
{
	if (empty())
		return false;

	if (!times) // if (times == 0) or assert(times)
		return false;

	bool res;
	switch (dir) {
	case COLUMNS_SHIFT_LEFT: // <--
		for ( unsigned int i = 0; i < times; i++) {
			for ( unsigned int j = 0; j < height_; j++) {
					gint16 tmp = data_[j * width_];

					for( unsigned int k = 0; k < width_ - 1; k++)
						data_[j * width_ + k] = data_[j * width_ + k + 1];

					data_[(j + 1) * width_ - 1] = tmp;
			}
		}
		res = true;
		break;
	case COLUMNS_SHIFT_RIGHT: // -->
		for ( unsigned int i = 0; i < times; i++) {
			for ( unsigned int j = 0; j < height_; j++) {
				gint16 tmp = data_[(j + 1) * width_ - 1];
				
				for( unsigned int k = width_ - 1; k != 0; k--)
					data_[j * width_ + k] = data_[j * width_ + k - 1];

				data_[j * width_] = tmp;
			}
		}
		res = true;
		break;
	default:
		res = false;
		break;
	}

	return res;
}

bool
Data::fix_strip(unsigned int pos)
{
	if (empty())
		return false;

	if (!pos || pos > width_) // assert(pos >= 1 && pos <= width_)
		return false;

	if(pos == 1) {
		for ( unsigned int i = 0; i < height_; i++)
			data_[i * width_] = data_[i * width_ + 1];
	}
	else if(pos == width_) {
		for( unsigned int i = 0; i < height_; i++)
			data_[i * width_ + (pos - 1)] = data_[i * width_ + (pos - 2)];
	}
	else {
		for( unsigned int i = 0; i < height_; i++) {
			double tmp = (data_[i * width_ + (pos - 2)] +
				data_[i * width_ + pos]) / 2.;

			data_[i * width_ + (pos - 1)] = static_cast<gint16>(tmp);
		}
	}

	return true;
}

bool
Data::fix_strips(const std::vector<guint>& bad_strips)
{
	std::vector<guint> all_strips(width_);

	for ( unsigned int i = 0; i < width_; ++i)
		all_strips[i] = i + 1;

	std::vector<guint>::const_iterator iter;
	std::vector<guint>::const_iterator end = all_strips.end();
	for ( iter = bad_strips.begin(); iter != bad_strips.end(); ++iter)
		end = std::remove( all_strips.begin(), all_strips.end(), *iter);

	int n = (bad_strips.size()) ? end - all_strips.begin() - bad_strips.size() :
		all_strips.size() - 1;

	double* x = new double[width_];
	double* y = new double[width_];
	double* p = new double[width_];

	unsigned int k = 0;
 	for ( iter = all_strips.begin(); iter != end; ++iter, ++k)
		x[k] = *iter;

	for ( unsigned int i = 0; i < height_; ++i) {
 		for ( k = 0, iter = all_strips.begin(); iter != end; ++iter, ++k)
			y[k] = pixel( (*iter - 1), i);

		ccm_cspl( x, y, p, n, tension);

		for ( iter = bad_strips.begin(); iter != bad_strips.end(); ++iter)
			pixel( (*iter - 1), i) =
				ccm_splfit( double(*iter), x, y, p, n, tension);
	}

	delete [] x;
	delete [] y;
	delete [] p;

	// now fix broken margins strips
	all_strips.resize(end - all_strips.begin());

	guint pos = *all_strips.begin();
	if (pos != 1) {
		for ( guint i = 1; i < pos; ++i)
			copy_strip( pos, i);
	}

	pos = *(--all_strips.end());
	if (pos != width_) {
		for ( guint i = pos + 1; i < width_ + 1; ++i)
			copy_strip( pos, i);
	}

	return true;
}

bool
Data::copy_strip( unsigned int from, unsigned int to)
{
	if (empty())
		return false;

	if (!to || to > width_) // assert(to >= 1 && to <= width_);
		return false;

	if (!from || from > width_) // assert (from >= 1 && from <= width_)
		return false;

	for( unsigned int i = 0; i < height_; i++)
		data_[i * width_ + (to - 1)] = data_[i * width_ + (from - 1)];

	return true;
}

bool
Data::swap_strips( unsigned int from, unsigned int to)
{
	if (empty())
		return false;

	if (!to || to > width_) // assert(to >= 1 && to <= width_);
		return false;

	if (!from || from > width_) // assert (from >= 1 && from <= width_)
		return false;

	for( unsigned int i = 0; i < height_; i++) {
		gint16 tmp = data_[i * width_ + (to - 1)];
		data_[i * width_ + (to - 1)] = data_[i * width_ + (from - 1)];
		data_[i * width_ + (from - 1)] = tmp;
	}

	return true;
}

DataVector
Data::mean_row( unsigned int lower, unsigned int upper) const
{
	if (empty())
		return DataVector();

	// assert(lower < height_ && upper < height_);
	if (lower >= height_ || upper >= height_)
		return DataVector();

	// assert(lower < upper);
	if (lower >= upper)
		return DataVector();
	
	DataVector row(width_);
	for ( unsigned int i = 0; i < width_; i++) {
		long sum = 0L;

		for ( unsigned int j = lower; j < upper + 1; j++)
			sum += data_[width_ * j + i];

		row[i] = static_cast<gint16>(sum / (upper - lower + 1));
	}
	return row;
}

DataVector
Data::mean_row() const
{
	if (empty())
		return DataVector();

	DataVector row(width_);
	for ( unsigned int i = 0; i < width_; i++) {
		long sum = 0L;

		for ( unsigned int j = 0; j < height_; j++)
			sum += data_[width_ * j + i];

		row[i] = static_cast<gint16>(sum / height_);
	}
	return row;
}

bool
Data::subtract_row(const DataVector& row)
{
	if (empty())
		return false;

	//assert(row.size() == width_);

	if (row.size() != width_)
		return false;

	for ( unsigned int i = 0; i < height_; ++i) {
		for ( unsigned int j = 0; j < width_; ++j) {
			data_[width_ * i + j] -= row[j];
		}
	}
	return true;
}

/**
 * [r1, r2)
*/
DataSharedPtr
Data::get_horizontal_part( unsigned int r1, unsigned int r2) const
{
	DataSharedPtr shared;

	if (empty())
		return shared;

	//assert(r2 > r1 && r2 <= height_);
	if (r2 <= r1 || r2 > height_)
		return shared;

	shared = create( width_, (r2 - r1));

	for( unsigned int i = 0; i < width_; i++) {
		for( unsigned int j = r1, k = 0; j < r2; j++, k++)
			shared->pixel( i, k) = data_[width_ * j + i];
	}

	return shared;
}

/**
 * [c1, c2)
*/
DataSharedPtr
Data::get_vertical_part( unsigned int c1, unsigned int c2) const
{
	DataSharedPtr shared;

	if (empty())
		return shared;

	//assert(c2 > c1 && c2 <= width_);
	if (c2 <= c1 || c2 > width_)
		return shared;

	shared = create((c2 - c1), height_);

	for ( unsigned int i = 0; i < height_; i++) {
		for ( unsigned int j = c1, k = 0; j < c2; j++, k++)
			shared->pixel( k, i) = data_[width_ * i + j];
	}

	return shared;
}

/**
 * [c1, c2)
*/
bool
Data::set_vertical_part( const DataSharedPtr& obj,
	unsigned int c1, unsigned int c2)
{
	if (empty())
		return false;

	//assert(c2 > c1 && c2 <= width_);
	if (c2 <= c1 || c2 > width_)
		return false;

	for ( unsigned int i = 0; i < height_; i++) {
		for ( unsigned int j = c1, k = 0; j < c2; j++, k++)
			data_[width_ * i + j] = obj->pixel( k, i);
	}

	return true;
}

bool
Data::add_value(gint16 value)
{
	if (empty())
		return false;

	std::transform( data_, data_ + width_ * height_, data_,
		std::bind2nd( std::plus<gint16>(), value));

	return true;
}

bool
Data::set_levels( double lower, double upper, double gamma)
{
	if (empty())
		return false;

	try {
		Magick::Image image( width_, height_, "I", Magick::ShortPixel, data_);

		image.level( lower, upper, gamma);

		const Magick::PixelPacket* pixels = image.getConstPixels( 0, 0,
			image.columns(), image.rows());

		for ( size_t i = 0; i < image.columns() * image.rows(); ++i) {
			double pix = double(SCANNER_ADC_COUNT_MAX * pixels++->red) / USHRT_MAX;
			pixel(i) = static_cast<gint16>(pix);
		}
	}
	catch (const Magick::Exception& ex) {
		return false;
	}
	return true;
}

bool
Data::normalize()
{
	if (empty())
		return false;

	std::transform( data_, data_ + width_ * height_, data_, normalize_value);

	return true;
}

std::ostream&
operator<<( std::ostream& s, const DataSharedPtr& obj)
{
	if (!obj)
		return s;

	s.write( (char *)&obj->width_, sizeof(short));
	s.write( (char *)&obj->height_, sizeof(short));
	s.write( (char *)obj->data_, sizeof(gint16) * obj->width_ * obj->height_);

	return s;
}

std::istream&
operator>>( std::istream& s, DataSharedPtr& obj)
{
	short w, h;

	s.read( (char *)&w, sizeof(short));
	s.read( (char *)&h, sizeof(short));

	//assert(w > 0 && h > 0);
	if (w && h) {
		gint16* data = new gint16[w * h];

		s.read( (char *)data, w * h * sizeof(gint16));

		obj = Data::create_from_data( w, h, data);

		delete [] data;
	}

	return s;
}

} // namespace Image

} // namespace ScanAmati
