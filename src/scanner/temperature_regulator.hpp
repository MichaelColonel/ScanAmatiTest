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

#include <deque>
#include <vector>
#include <glibmm/timer.h>

#include <config.h>

#ifdef HAVE_GLIB_2_0
#include <glib.h>
#endif

#define SCANNER_DEFAULT_TEMPERATURE_AVERAGE 7.
#define SCANNER_DEFAULT_TEMPERATURE_SPREAD 0.2

namespace ScanAmati {

namespace Scanner {

union AdcCount;

template<typename T>
class FixedStack : public std::deque<T> {
public:
	FixedStack(size_t size) : std::deque<T>(), size_(size) {}
	virtual ~FixedStack() { std::deque<T>::clear(); }
	void push_front(const T& v);

protected:
	size_t size_;
};

typedef FixedStack<double> TemperatureStack;

union AdcCount;

class TemperatureRegulator : public TemperatureStack {
friend class Manager;
public:
	enum {
		TEMPERATURE_RANGE = 3,
		CODES_RANGE = 3,
		TEMPERATURE_FIT_DIMENTION = 3
	};
	TemperatureRegulator(size_t size)
		:
		TemperatureStack(size),
		data_loaded_(false),
		code_value_changed_(false),
		temperature_codes_( CODES_RANGE, UCHAR_MAX) {}
	void set_margins( double average, double spread);
	void set_default_margins();
	bool load_data(const std::string&);
	void reset();
	guint8 add(double); // add temperature
	double temperature(const AdcCount&) const; // current temperature
	bool code_value_changed() const { return code_value_changed_; }
	static bool temperature_within_range(double t);

protected:
	virtual guint8 base_diff_code() const;
	bool data_loaded_;
	guint8 code_;
	Glib::Timer timer_;
	bool code_value_changed_;
	std::vector<int> temperature_codes_;
	std::vector<double> temperature_fit_coefficients_;

	static std::vector<double> temperature_margins_;
};

inline
bool
TemperatureRegulator::temperature_within_range(double t)
{
	return (t >= temperature_margins_[0] && t <= temperature_margins_[2]);
}

inline
void
TemperatureRegulator::set_margins( double average, double spread)
{
	temperature_margins_.clear();
	temperature_margins_.push_back(average - spread);
	temperature_margins_.push_back(average);
	temperature_margins_.push_back(average + spread);
}

inline
void
TemperatureRegulator::set_default_margins()
{
	set_margins( SCANNER_DEFAULT_TEMPERATURE_AVERAGE,
		SCANNER_DEFAULT_TEMPERATURE_SPREAD);
}

} // namespace Scanner

} // namespace ScanAmati
