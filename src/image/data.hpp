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

#include <glib.h>

#include <vector>
#include <tr1/memory>
#include <fstream>


namespace ScanAmati {

namespace Image {

class Data;

typedef std::vector<gint16> DataVector;

typedef std::tr1::shared_ptr<Data> DataSharedPtr;
typedef std::tr1::weak_ptr<Data> DataWeakPtr;

class Data {

friend std::ostream& operator<<( std::ostream&, const DataSharedPtr&);
friend std::istream& operator>>( std::istream&, DataSharedPtr&);

public:
	enum ColumnsShiftDirectionType {
		COLUMNS_SHIFT_LEFT, // <--
		COLUMNS_SHIFT_RIGHT // -->
	};
	virtual ~Data();

	bool empty() const;
	void clear();
	unsigned int width() const { return width_; }
	unsigned int height() const { return height_; }

	bool shift_columns( ColumnsShiftDirectionType dir = COLUMNS_SHIFT_LEFT,
		unsigned int times = 1);

	bool fix_strip(unsigned int pos); /**< pos from 1 to width */
	bool fix_strips(const std::vector<guint>& bad_strips);
	bool subtract_row(const DataVector& row);
	bool copy_strip( unsigned int from, unsigned int to);
	bool swap_strips( unsigned int from, unsigned int to);
	bool add_value(gint16 value);

	DataSharedPtr get_horizontal_part( unsigned int begin,
		unsigned int end) const; /**< [begin, end) */
	DataSharedPtr get_vertical_part( unsigned int left,
		unsigned int right) const; /**< [left, right) */

	DataVector mean_row() const;
	DataVector mean_row( unsigned int lower,
		unsigned int upper) const;

	bool set_vertical_part( const DataSharedPtr& obj,
		unsigned int left, unsigned int right);
	bool normalize();

	bool set_levels( double lower, double upper, double gamma);

	gint16& pixel( unsigned int column, unsigned int row);
	gint16& pixel( unsigned int column, unsigned int row) const;
	gint16& pixel(unsigned int pos) { return *(data_ + pos); }
	gint16& pixel(unsigned int pos) const { return *(data_ + pos); }
	gint16* data() { return data_; }
	const gint16* data() const { return data_; }
	gint16* begin() { return data_; }
	const gint16* begin() const { return data_; }
	gint16* end() { return data_ + width_ * height_; }
	const gint16* end() const { return data_ + width_ * height_; }
	bool image_buffer(std::vector<guint8>&) const;

	static DataSharedPtr create( unsigned int width, unsigned int height);
	static DataSharedPtr create_from_data( unsigned int width,
		unsigned int height, const gint16* data);
	static DataSharedPtr create_from_shared(const DataSharedPtr& shared);

protected:
	Data() : width_(0), height_(0), data_(0) {}
	Data( unsigned int width, unsigned int height);
	Data( unsigned int width, unsigned int height, const gint16* data);
	Data( unsigned int width, unsigned int height, const DataVector& data);

	Data(const Data& obj);
	Data& operator=(const Data& obj);

	unsigned int width_;
	unsigned int height_;
	gint16* data_;
};

inline
gint16&
Data::pixel( unsigned int column, unsigned int row)
{
	return *(data_ + row * width_ + column);
}

inline
gint16&
Data::pixel( unsigned int column, unsigned int row) const
{
	return *(data_ + row * width_ + column);
}

inline
bool
Data::empty() const
{
	return !(width_ && height_ && data_);
}

inline
void
Data::clear()
{
	if (!empty()) {
		delete [] data_;
		width_ = 0;
		height_ = 0;
		data_ = 0;
	}
}

} // namespace Image

} // namespace ScanAmati
