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

#include <cmath>
#include <fstream>
#include <iostream>
#include <algorithm>

#include <config.h> /* for DEBUG */

/* files from src directory begin */
#include "ccmath_wrapper.h"
#include "application.hpp"
/* files from src directory end */

#include "adc_count.hpp"
#include "temperature_regulator.hpp"

namespace {

struct TemperatureAndCounts {
	TemperatureAndCounts(size_t size)
		:
		counts( size, 0),
		temperature( size, 0) {}
	std::vector<int> counts;
	std::vector<double> temperature;
};

const double default_temperature_margins[] = {
	SCANNER_DEFAULT_TEMPERATURE_AVERAGE - SCANNER_DEFAULT_TEMPERATURE_SPREAD,
	SCANNER_DEFAULT_TEMPERATURE_AVERAGE,
	SCANNER_DEFAULT_TEMPERATURE_AVERAGE + SCANNER_DEFAULT_TEMPERATURE_SPREAD
};

} // namespace

namespace ScanAmati {

namespace Scanner {

template<typename T>
void
FixedStack<T>::push_front(const T& v)
{
	if (std::deque<T>::size() == size_)
		std::deque<T>::pop_back();

	std::deque<T>::push_front(v);
}

std::vector<double>
TemperatureRegulator::temperature_margins_( default_temperature_margins,
	default_temperature_margins + G_N_ELEMENTS(default_temperature_margins));

bool
TemperatureRegulator::load_data(const std::string& filename)
{
	if (filename.empty()) {
		data_loaded_ = false;
		return false;
	}

	std::ifstream file(filename.c_str());

	if (!file.is_open()) {
		data_loaded_ = false;
		return false;
	}

	for (int i = 0; i < CODES_RANGE; ++i)
	file >> temperature_codes_[i];

	int n;
	file >> n;

	if (n > 0) {
		temperature_fit_coefficients_.resize(TEMPERATURE_FIT_DIMENTION);
		std::vector<double> tmp(TEMPERATURE_FIT_DIMENTION * n);
		TemperatureAndCounts temperature_and_counts(n);

		for ( int i = 0; i < n; ++i)
			file >> temperature_and_counts.counts[i];

		for ( int i = 0; i < n; ++i)
			file >> temperature_and_counts.temperature[i];

		for ( int i = 0; i < n; ++i) {
			for ( int j = 0; j < TEMPERATURE_FIT_DIMENTION; ++j)
				tmp[i * TEMPERATURE_FIT_DIMENTION + j] =
					pow( temperature_and_counts.counts[i], j);
		}

		if (app.debug) {
			std::cout << "Compute least squares coefficients via QR reduction.";
			std::cout << std::endl;
			ccm_matprt( &tmp[0], n, TEMPERATURE_FIT_DIMENTION, " %9.4f");
		}

		// compute least squares coefficients via QR reduction
		int i;
		double t = ccm_qrlsq( &tmp[0], &temperature_and_counts.temperature[0],
			n, TEMPERATURE_FIT_DIMENTION, &i);
		if (i == -1) {
			std::cerr << "Singular reduced matrix." << std::endl;
			data_loaded_ = false;
		}
		else {
			if (app.debug)
				printf( "Sum of squared fit residuals (ssq) = %.3e.\n", t);

			std::copy( temperature_and_counts.temperature.begin(),
				temperature_and_counts.temperature.begin() + TEMPERATURE_FIT_DIMENTION,
				temperature_fit_coefficients_.begin());
			data_loaded_ = true;
		}
	}

	file.close();
	
	return data_loaded_;
}

unsigned char
TemperatureRegulator::add(double temp)
{
	push_front(temp);

	if (size() == 1) {
		code_ = temperature_codes_[1];
		timer_.start();
		return code_;
	}

	double diff = front() - *++begin();
	guint8 diff_code = base_diff_code();

	if (temp <= temperature_margins_[2] && temp >= temperature_margins_[0])
		code_value_changed_ = false;
	else if (temp < temperature_margins_[0] && timer_.elapsed() > 15) {
		if (diff < 0 && code_ < temperature_codes_[2]) {
			if (int(code_) + diff_code > temperature_codes_[2]) {
				code_ = temperature_codes_[2];
				code_value_changed_ = true;
				timer_.start();
			}
			else if (code_ == temperature_codes_[2])
				code_value_changed_ = false;
			else {
				code_ += diff_code;
				code_value_changed_ = true;
				timer_.start();
			}
			return code_;
		}
		else
			code_value_changed_ = false;
	}
	else if (temp > temperature_margins_[2] && timer_.elapsed() > 15) {
		if (diff > 0 && code_ > temperature_codes_[0]) {
			if (int(code_) - diff_code < temperature_codes_[0]) {
				code_ = temperature_codes_[0];
				code_value_changed_ = true;
				timer_.start();
			}
			else if (code_ == temperature_codes_[0])
				code_value_changed_ = false;
			else {
				code_ -= diff_code;
				code_value_changed_ = true;
				timer_.start();
			}
			return code_;
		}
		else
			code_value_changed_ = false;
	}
	else
		code_value_changed_ = false;

	return code_;
}

guint8
TemperatureRegulator::base_diff_code() const
{
	double diff_base = fabs( front() - temperature_margins_[1]);

	guint8 code;
	if (diff_base > 2)
		code = 5;
	else if (diff_base <= 2 && diff_base > 1)
		code = 2;
	else if (diff_base <= 1 && diff_base > 0.2)
		code = 1;
	else
		code = 1;

	return code;
}

double
TemperatureRegulator::temperature(const AdcCount& count) const
{
	double t = 0.;
	for ( int i = 0; i < TEMPERATURE_FIT_DIMENTION; ++i)
		t += temperature_fit_coefficients_[i] * pow( count.temperature_code(), i);

	return t;
}

} // namespace Scanner

} // namespace ScanAmati
