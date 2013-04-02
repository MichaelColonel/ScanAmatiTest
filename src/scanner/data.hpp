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

#include <tr1/tuple>

#include <glibmm/thread.h>
#include <glibmm/dispatcher.h>

#include <Magick++/Include.h>

#include "assemble.hpp"

namespace boost {
class any;
} // namespace boost

namespace ScanAmati {
	
namespace Scanner {

enum DataType {
	DATA_RAW,
	DATA_CALIBRATED
};

enum LiningAccuracyType {
	LINING_ACCURACY_ROUGH = 20,
	LINING_ACCURACY_OPTIMAL = 51,
	LINING_ACCURACY_PRECISE = 255
};

enum CalibrationType {
	CALIBRATION_ROUGH = 50,
	CALIBRATION_GOOD = 100,
	CALIBRATION_BETTER = 200,
	CALIBRATION_BEST = 500
};

enum WidthType {
	WIDTH_FULL,
	WIDTH_HALF,
	WIDTH_QUARTER
};

enum AcquireType {
	ACQUIRE_IMAGE,
	ACQUIRE_IMAGE_PEDESTALS,
	ACQUIRE_LINING_PEDESTALS
};

enum PixelIntensityType {
	INTENSITY_ORIGINAL,
	INTENSITY_LINEAR,
	INTENSITY_LOGARITHMIC
};

typedef std::pair< WidthType, CalibrationType> WidthCalibrationPair;
typedef std::pair< guint8, gint16> LiningPair;

class Data {
	friend class Manager;

public:
	Data();
	virtual ~Data();
	Glib::Dispatcher& signal_complete() { return signal_complete_; }
	void run_image_reconstruction(const boost::any& arg);
	Image::SummaryData get_summary_data() { return image_data_; }
	void set_chip_lining( char chip_code, const std::vector<guint8>& lining);
	void set_lining(const std::map< char, std::vector<guint8> >& lining);
	AssemblyConstIter begin_assemble() { return assembly_.begin(); }
	AssemblyConstIter end_assemble() { return assembly_.end(); }

	static guint chip_number(char code);
	static char chip_code(guint number);

private:
	void reconstruct_image_from_scratch();
	void reconstruct_image_intensity(PixelIntensityType intensity);
	void reconstruct_image_calibration(CalibrationType accuracy);
	void reconstruct_image_levels(std::vector<double> levels);
	void reconstruct_image_filter(Magick::FilterTypes filter);
	void reconstruct_image_width(WidthType width);
	void reconstruct_image_data(DataType data_type);

	bool load_bad_strips(const std::string& filename);
	bool save_lining(const std::string& filename) const;
	bool load_lining(const std::string& filename);

	void reconstruct( AcquireType acquire, guint8 arg);
	void reconstruct_pedestals( AcquireType acquire,
		const std::vector<Image::DataSharedPtr>& array, guint8 arg);
	void reconstruct_image(const std::vector<Image::DataSharedPtr>& array);

	void preprocess(AcquireType acquire_type);
	Image::DataSharedPtr image_from_memory(AcquireType acquire) const;
	std::vector<Image::DataSharedPtr> form_assembly_array(
		const Image::DataSharedPtr& image) const;

	void calculate_lining(guint8 accuracy);

	CodeCountsMap expand_lining_code_counts(
		const CodeCountsMap& map) const;

	std::vector<guint> form_bad_strips(WidthType) const;
	Image::DataSharedPtr form_image(WidthType width_type = WIDTH_FULL);

	void fill_image_data( Image::DataSharedPtr& raw_image,
		PixelIntensityType intensity);

	bool check_image_data( AssemblyConstIter begin,
		AssemblyConstIter end) const;

	Image::DataSharedPtr calibrate( const Image::DataSharedPtr& raw,
		const std::vector<guint>& bad_strips,
		CalibrationType calibration_type = CALIBRATION_ROUGH);

	void width_iterators( WidthType width_type, AssemblyConstIter& begin,
		AssemblyConstIter& end) const;
	void width_iterators( WidthType width_type, AssemblyIter& begin,
		AssemblyIter& end);

	void clear();

	guint8* memory_;
	AssemblyVector assembly_;

	guint data_offset_;
	guint chip_offset_;

	unsigned int image_height_;
	size_t memory_size_;
	Magick::FilterTypes filter_type_;
	WidthType width_type_;
	CalibrationType calibration_type_;
	PixelIntensityType intensity_type_;
	gint16 lining_count_;

	Glib::Thread* thread_;
	Glib::Dispatcher signal_complete_;
	Image::SummaryData image_data_;
	union RawData {
		RawData() : memory(0) {}
		guint8* memory;
		guint16* counts;
	} AdcData_;
};

} // namespace Scanner

} // namespace ScanAmati
