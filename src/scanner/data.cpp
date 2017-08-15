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
#include <sstream>

#include <unistd.h>
#include <cmath>

#include <Magick++.h>

#include <boost/any.hpp>

#include "adc_count.hpp"
#include "data.hpp"

/* files from src directory begin */
#include "global_strings.hpp"
#include "application.hpp"
/* files from src directory end */

namespace {

enum PosType {
	BEGIN,
	VALUE,
	END
};

struct Margin {
	int start;
	int end;
	PosType pos;
	double value;
};

bool
not_null(bool value)
{
	return value;
}

} // namespace

namespace ScanAmati {

namespace Scanner {

Data::Data()
	:
	memory_(0),
	assembly_(SCANNER_CHIPS),
	data_offset_(0),
	chip_offset_(0),
	image_height_(IMAGE_HEIGHT),
	memory_size_(SCANNER_MEMORY),
	filter_type_(Magick::CubicFilter),
	width_type_(WIDTH_FULL),
	calibration_type_(CALIBRATION_GOOD),
	intensity_type_(INTENSITY_ORIGINAL),
	lining_count_(SCANNER_LINING_COUNT)
{
	memory_ = new guint8[SCANNER_MEMORY_ALL];

	const char* code = array_chip_codes;

	for ( AssemblyIter it = assembly_.begin(); it != assembly_.end(); ++it)
		it->code = *code++;
}

Data::~Data()
{
	delete [] memory_;
}

guint
Data::chip_number(char code)
{
	const char* start = array_chip_codes;
	const char* end = array_chip_codes + SCANNER_CHIPS;

	const char* pos = std::find( start, end, code);
	return (pos - array_chip_codes);
}

char
Data::chip_code(guint number)
{
	return (number < SCANNER_CHIPS) ? array_chip_codes[number] : 0;
}

void
Data::width_iterators( WidthType width_type,
	AssemblyConstIter& begin, AssemblyConstIter& end) const
{
	begin = assembly_.begin();
	end = assembly_.end();
	switch (width_type) {
	case WIDTH_HALF:
		begin += SCANNER_CHIPS_DROP_HALF;
		end -= SCANNER_CHIPS_DROP_HALF;
		break;
	case WIDTH_QUARTER:
		begin += SCANNER_CHIPS_DROP_QUARTER;
		end -= SCANNER_CHIPS_DROP_QUARTER;
		break;
	case WIDTH_FULL:
	default:
		break;
	}
}

void
Data::width_iterators( WidthType width_type,
	AssemblyIter& begin, AssemblyIter& end)
{
	begin = assembly_.begin();
	end = assembly_.end();
	switch (width_type) {
	case WIDTH_HALF:
		begin += SCANNER_CHIPS_DROP_HALF;
		end -= SCANNER_CHIPS_DROP_HALF;
		break;
	case WIDTH_QUARTER:
		begin += SCANNER_CHIPS_DROP_QUARTER;
		end -= SCANNER_CHIPS_DROP_QUARTER;
		break;
	case WIDTH_FULL:
	default:
		break;
	}
}

bool
Data::check_image_data( AssemblyConstIter begin,
	AssemblyConstIter end) const
{
	bool res = true;
	for ( AssemblyConstIter it = begin; it != end; ++it) {

		if (it->raw_data && !it->raw_data->empty())
			continue;

		OFLOG_DEBUG( app.log,
			"Raw data in the assemble " << it->code << " is invalid");

		res = false;
	}
	return res;
}

bool
Data::save_lining(const std::string& filename) const
{
	std::ofstream file(filename.c_str());
	if (file.is_open()) {
		for ( AssemblyConstIter iter = assembly_.begin();
			iter != assembly_.end(); ++iter) {
			file << iter->code;
			for ( std::vector<guint8>::const_iterator it = iter->lining.begin();
				it != iter->lining.end(); ++it)
				file << " " << int(*it);
			file << std::endl;
		}
		file.close();
	}
	else
		return false;

	return true;
}

bool
Data::load_lining(const std::string& filename)
{
	std::ifstream file(filename.c_str());
	if (file.is_open()) {
		for ( AssemblyIter it = assembly_.begin(); it != assembly_.end();
			++it) {
			file >> it->code;

			for ( std::vector<guint8>::size_type i = 0; i < it->lining.size();
				++i) {
				int value;
				file >> value;
				it->lining[i] = static_cast<guint8>(value);
			}
		}
		file.close();
	}
	else
		return false;

	return true;
}

bool
Data::load_bad_strips(const std::string& filename)
{
	std::ifstream file(filename.c_str());

	std::map< char, std::vector<guint> > map;
	if (file.is_open()) {
		std::vector<guint> strips;
		char code = -1;
		char peek = file.peek();

		if (peek != '\n')
			file >> code;

		while (!file.eof()) {
			if (peek == '\n') {
				map[code] = strips;
				strips.clear();
				file >> code;
			}
			else {
				guint strip;
				file >> strip;
				strips.push_back(strip);
			}
			peek = file.peek();
		}
		file.close();
	}
	else
		return false;

	for ( AssemblyIter it = assembly_.begin(); it != assembly_.end(); ++it)
		it->bad_strips = map[it->code];

	return true;
}

void
Data::preprocess(AcquireType acquire_type)
{
	AdcData_.memory = memory_ + 1;

	switch (acquire_type) {
	case ACQUIRE_IMAGE_PEDESTALS:
	case ACQUIRE_LINING_PEDESTALS:
		AdcData_.memory += SCANNER_MEMORY;
		break;
	case ACQUIRE_IMAGE:
	default:
		break;
	}

	// find data offset
	for ( guint i = 0; i < SCANNER_STRIPS; ++i) {
		if (AdcCount(AdcData_.counts[i]).data_bit()) {
			data_offset_ = i;
			break;
		}
	}

	// find chip offset (data from memory)
	std::vector<bool> row(SCANNER_STRIPS);
	for ( int j = 0; j < SCANNER_STRIPS_PER_CHIP; ++j) {
		for ( int i = 0; i < SCANNER_CHIPS; ++i) {
			int dest = i * SCANNER_STRIPS_PER_CHIP + j; // row pos
			int src = j * SCANNER_CHIPS + i + data_offset_; // memory pos

			row[dest] = AdcCount(AdcData_.counts[src]).chip_bit();
		}
	}

	std::vector<bool>::const_reverse_iterator pos = std::find_if( row.rbegin(),
		row.rend(), not_null);

	chip_offset_ = (pos - row.rbegin()) / SCANNER_STRIPS_PER_CHIP;
	chip_offset_ += 1;
}

Image::DataSharedPtr
Data::image_from_memory(AcquireType acquire) const
{
	Image::DataSharedPtr image;

	unsigned int rows = IMAGE_ROWS;

	switch (acquire) {
	case ACQUIRE_IMAGE:
		rows = (((memory_size_ >> 1) / SCANNER_STRIPS) - 2);
		break;
	case ACQUIRE_IMAGE_PEDESTALS:
	case ACQUIRE_LINING_PEDESTALS:
		rows = (((SCANNER_MEMORY_PART >> 1) / SCANNER_STRIPS) - 2);
		break;
	default:
		break;
	}

	image = Image::Data::create( SCANNER_STRIPS, rows);

	// data from memory
	for ( unsigned int k = 0; k < rows; ++k) {
		for ( unsigned int j = 0; j < SCANNER_STRIPS_PER_CHIP; ++j) {
			for ( unsigned int i = 0; i < SCANNER_CHIPS; ++i) {
				unsigned int dest = i * SCANNER_STRIPS_PER_CHIP + j + // column
					k * SCANNER_STRIPS; // row
				unsigned int src = j * SCANNER_CHIPS + i + // column
					k * SCANNER_STRIPS + data_offset_; // row

				image->pixel(dest) = AdcCount(AdcData_.counts[src]).pixel();
			}
		}
	}

	// Shift first strip for the first four assembly
	for ( unsigned int i = 0; i < SCANNER_CHIPS; ++i) {
		// skip assembly [4, 15]
		if (i > 3)
			continue;

		unsigned int from = i * SCANNER_STRIPS_PER_CHIP;
		unsigned int to = (i + 1) * SCANNER_STRIPS_PER_CHIP;
		Image::DataSharedPtr data = image->get_vertical_part( from, to);
		data->shift_columns();
		image->set_vertical_part( data, from, to);
	}

	return image;
}

std::vector<Image::DataSharedPtr>
Data::form_assembly_array(const Image::DataSharedPtr& image) const
{
	// assembly array
	std::vector<Image::DataSharedPtr> array(SCANNER_CHIPS);

	// drop service strips
	guint i = 0;
	std::vector<Image::DataSharedPtr>::iterator it;
	for ( it = array.begin(); it != array.end(); ++it, ++i) {
		int from = i * SCANNER_STRIPS_PER_CHIP;
		int to = i * SCANNER_STRIPS_PER_CHIP + IMAGE_STRIPS_PER_CHIP;
		*it = image->get_vertical_part( from, to);
	}

	// rotate to the start of the frame
	if (chip_offset_ != SCANNER_CHIPS) {
		guint offset = SCANNER_CHIPS - chip_offset_;
		std::rotate( array.begin(), array.begin() + offset, array.end());
	}

	return array;
}

void
Data::reconstruct( AcquireType acquire, guint8 arg)
{
	// preprocess
	preprocess(acquire);

	// forms raw image
	Image::DataSharedPtr image = image_from_memory(acquire);

	// drop service strips, rotate to the start of the frame, write to assembly
	std::vector<Image::DataSharedPtr> array = form_assembly_array(image);

	switch (acquire) {
	case ACQUIRE_IMAGE:
		{
			std::ofstream file("/tmp/raw_image.raw");
			file << image;
			file.close();
		}
		reconstruct_image(array);
		break;
	case ACQUIRE_IMAGE_PEDESTALS:
	case ACQUIRE_LINING_PEDESTALS:
		reconstruct_pedestals( acquire, array, arg);
		break;
	default:
		break;
	}
}

void
Data::reconstruct_pedestals( AcquireType acquire,
	const std::vector<Image::DataSharedPtr>& array,
	guint8 arg)
{
	guint i = 0;
	AssemblyIter it;
/*
	std::stringstream ss;
	ss << int(arg) << ".raw";
	std::cout << ss.str() << std::endl;
	std::ofstream file(ss.str().c_str());
	file << array[0];
	file.close();
*/
	for ( it = assembly_.begin(); it != assembly_.end(); ++it, ++i) {
		switch (acquire) {
		case ACQUIRE_IMAGE_PEDESTALS:
			it->pedestals = array[i]->mean_row();
			break;
		case ACQUIRE_LINING_PEDESTALS:
			{
				guint8& code = arg;
				Image::DataVector counts = array[i]->mean_row();
				it->code_counts_map.insert(CodeCountsPair( code, counts));
			}
			break;
		case ACQUIRE_IMAGE:
			break;
		default:
			break;
		}
	}
}

void
Data::reconstruct_image(const std::vector<Image::DataSharedPtr>& array)
{
	// copy array data to assembly vector
	guint i = 0;
	AssemblyIter it;
	for ( it = assembly_.begin(); it != assembly_.end(); ++it, ++i) {
		if (it->pedestals.size()) {
			array[i]->subtract_row(it->pedestals);
			array[i]->add_value(lining_count_);
			array[i]->normalize();
		}

		const unsigned int& rows = image_height_;
		// resize Image
		Magick::Image image( IMAGE_STRIPS_PER_CHIP, array[i]->height(), "I",
			Magick::ShortPixel, array[i]->data());

		Magick::Geometry geom( IMAGE_STRIPS_PER_CHIP, rows);
		geom.aspect(true);
		image.filterType(filter_type_);
//#if (MagickLibVersion >= 0x660 && MagickLibVersion <= 0x669) 
		image.resize(geom);
//#else
//		image.scale(geom);
//#endif

		const Magick::PixelPacket* pixel = image.getConstPixels( 0, 0,
			IMAGE_STRIPS_PER_CHIP, rows);

		it->raw_data = Image::Data::create( IMAGE_STRIPS_PER_CHIP, rows);
		for ( unsigned int i = 0; i < IMAGE_STRIPS_PER_CHIP * rows; ++i)
			it->raw_data->pixel(i) = pixel++->red; // pixel->red; ++pixel;
	}
}

void
Data::run_image_reconstruction(const boost::any& arg)
{
	sigc::slot<void> slot;

	if (arg.type() == typeid(PixelIntensityType)) {
		PixelIntensityType intensity =
			boost::any_cast<PixelIntensityType>(arg);
		slot = sigc::bind(
			sigc::mem_fun( *this, &Data::reconstruct_image_intensity),
			intensity);
	}
	else if (arg.type() == typeid(Magick::FilterTypes)) {
		Magick::FilterTypes filter =
			boost::any_cast<Magick::FilterTypes>(arg);
		slot = sigc::bind(
			sigc::mem_fun( *this, &Data::reconstruct_image_filter),
			filter);
	}
	else if (arg.type() == typeid(std::vector<double>)) {
		std::vector<double> lvl =
			boost::any_cast< std::vector<double> >(arg);
		slot = sigc::bind(
			sigc::mem_fun( *this, &Data::reconstruct_image_levels),
			lvl);
	}
	else if (arg.type() == typeid(WidthType)) {
		WidthType width = boost::any_cast<WidthType>(arg);
		slot = sigc::bind(
			sigc::mem_fun( *this, &Data::reconstruct_image_width),
			width);
	}
	else if (arg.type() == typeid(DataType)) {
		DataType data_type = boost::any_cast<DataType>(arg);
		slot = sigc::bind(
			sigc::mem_fun( *this, &Data::reconstruct_image_data),
			data_type);
	}
	else if (arg.type() == typeid(CalibrationType)) {
		CalibrationType accuracy = boost::any_cast<CalibrationType>(arg);
		slot = sigc::bind(
			sigc::mem_fun( *this, &Data::reconstruct_image_calibration),
			accuracy);
	}
	else
		slot = sigc::mem_fun( *this, &Data::reconstruct_image_from_scratch);

	thread_ = Glib::Thread::create( slot, true);
}

void
Data::reconstruct_image_from_scratch()
{
	reconstruct( ACQUIRE_IMAGE, 0);

	Image::DataSharedPtr image = form_image(width_type_);
	if (image) {
		std::vector<guint> bs = form_bad_strips(width_type_);
//		Image::DataSharedPtr result = calibrate( image, bs, calibration_type_);
//		calibrated_image_ = result->get_horizontal_part( 0, margins[0].end);
//		image->normalize();
//		calibrated_image_ = image;
//		image->fix_strips(bs);
		fill_image_data( image, intensity_type_);
	}

	signal_complete_();
}

void
Data::reconstruct_image_filter(Magick::FilterTypes filter)
{
/*	filter_type_ = filter;

	reconstruct( ACQUIRE_IMAGE, 0);

	Image::DataSharedPtr image = form_image(width_type_);
	if (image) {		
		std::vector<guint> bs = form_bad_strips(width_type_);
//		Image::DataSharedPtr result = calibrate( image, bs, calibration_type_);
//		calibrated_image_ = result->get_horizontal_part( 0, margins[0].end);
		image->normalize();
		calibrated_image_ = image;
		form_image_tuple( calibrated_image_, intensity_type_);
	}
*/
	signal_complete_();
}

void
Data::reconstruct_image_intensity(PixelIntensityType intensity)
{
/*	intensity_type_ = intensity;
	Image::DataSharedPtr image =
		Image::Data::create_from_shared(calibrated_image_);
	if (image)
		form_image_tuple( image, intensity);

*/	signal_complete_();
}

void
Data::reconstruct_image_levels(std::vector<double> levels)
{
/*	Image::DataSharedPtr image =
		Image::Data::create_from_shared(calibrated_image_);
	
	if (image) {
		image->set_levels( levels[0], levels[1], levels[2]);
		form_image_tuple( image, intensity_type_);
	}
*/	signal_complete_();
}

void
Data::reconstruct_image_calibration(CalibrationType accuracy)
{
/*	calibration_type_ = accuracy;
	Image::DataSharedPtr image = form_image(width_type_);
	if (image) {		
		std::vector<guint> bs = form_bad_strips(width_type_);
		calibrated_image_ = calibrate( image, bs, accuracy);
		form_image_tuple( calibrated_image_, intensity_type_);
	}
*/	signal_complete_();
}

void
Data::reconstruct_image_width(WidthType width)
{
/*	width_type_ = width;
	Image::DataSharedPtr image = form_image(width);
	if (image) {		
		std::vector<guint> bs = form_bad_strips(width);
		calibrated_image_ = calibrate( image, bs, calibration_type_);
		form_image_tuple( calibrated_image_, intensity_type_);
	}
*/	signal_complete_();
}

void
Data::reconstruct_image_data(DataType data_type)
{
/*	Image::DataSharedPtr image;
	switch (data_type) {
	case DATA_RAW:
		image = form_image(width_type_);
		break;
	case DATA_CALIBRATED:
		image = calibrated_image_;
		break;
	default:
		break;
	}
	if (image) {
		fill_image_data( image, INTENSITY_ORIGINAL);
	}
*/
	signal_complete_();
}

void
Data::clear()
{
	for ( AssemblyIter it = assembly_.begin(); it != assembly_.end(); ++it)
		it->clear_after_disconnect();
}

Image::DataSharedPtr
Data::calibrate( const Image::DataSharedPtr& raw,
	const std::vector<guint>& bs,
	CalibrationType calibration_type)
{
	double begin, end, value;
	Image::DataSharedPtr clear;
	Image::CalibrationMap map;
	Image::Calibration calib( map, bs, calibration_type, 200.);
/*
	Image::DataVector vec( raw->width(), lining_count_);
	calib.add_row( lining_count_, vec);
	begin = double(lining_count_);

	int i = 0;
	while (margins[i].start != -1) {
		Image::DataVector v = raw->mean_row( margins[i].start, margins[i].end);
		value = calib.add_row( v, true);
		switch (margins[i].pos) {
		case END:
			end = value;
			break;
		case BEGIN:
			begin = value;
			break;
		case VALUE:
		default:
			break;
		}
		++i;
	}
	calib.expand( begin, end);
*/
	clear = calib.calibrate(raw);

	clear->fix_strips(bs);
	clear->normalize();
	return clear;
}

Image::DataSharedPtr
Data::form_image(WidthType width_type)
{
	Image::DataSharedPtr image;

	AssemblyConstIter begin, end;
	width_iterators( width_type, begin, end);

	if (check_image_data( begin, end)) {
		unsigned int width = (end - begin) * SCANNER_STRIPS_PER_CHIP_REAL;
		image = Image::Data::create( width, image_height_);

		for ( AssemblyConstIter it = begin; it != end; ++it) {
			guint i = it - begin;
			int from = i * IMAGE_STRIPS_PER_CHIP;
			int to = (i + 1) * IMAGE_STRIPS_PER_CHIP;
			image->set_vertical_part( it->raw_data, from, to);
		}
	}
	return image;
}

void
Data::fill_image_data( Image::DataSharedPtr& image,
	PixelIntensityType intensity_type)
{
	std::vector<guint8> buf(image->width() * image->height());

	gint16 min = *std::min_element( image->begin(), image->end());
	gint16 max = *std::max_element( image->begin(), image->end());

	for ( gint16* pos = image->begin(); pos != image->end(); ++pos) {
		ptrdiff_t i = pos - image->begin();
		switch (intensity_type) {
		case INTENSITY_LOGARITHMIC:
			{
				double mn = log(min + 1);
				double mx = log(max + 1);
				double value = (log(*pos + 1) - mn) / (mx - mn);
				buf[i] = static_cast<guint8>(UCHAR_MAX * value);
				*pos = static_cast<gint16>(SCANNER_ADC_COUNT_MAX * value);
			}
			break;
		case INTENSITY_LINEAR:
			{
				double value = double(*pos - min) / (max - min);
				buf[i] = static_cast<guint8>(UCHAR_MAX * value);
				*pos = static_cast<gint16>(SCANNER_ADC_COUNT_MAX * value);
			}
			break;
		case INTENSITY_ORIGINAL:
		default:
			{
				double value = double(UCHAR_MAX * *pos) / SCANNER_ADC_COUNT_MAX;
				buf[i] = static_cast<guint8>(value);
			}
			break;
		}
	}
	image_data_.raw_data() = image;
	image_data_.image_buffer() = buf;
}

std::vector<guint>
Data::form_bad_strips(WidthType width_type) const
{
	std::vector<guint> strips;

	AssemblyConstIter begin, end;
	width_iterators( width_type, begin, end);

	unsigned int start = chip_number(begin->code);
	for ( AssemblyConstIter iter = begin; iter != end; ++iter) {
		unsigned int pos = chip_number(iter->code);
		pos -= start;

		const std::vector<guint>& ref = iter->bad_strips;
		std::vector<guint>::const_iterator it;
		for ( it = ref.begin(); it != ref.end(); ++it) {
			unsigned int strip = pos * SCANNER_STRIPS_PER_CHIP_REAL + *it;
			strips.push_back(strip);
		}
	}
 
	return strips;
}

void
Data::calculate_lining(guint8 accuracy)
{
	for ( AssemblyIter it = assembly_.begin(); it != assembly_.end(); ++it) {
		if (accuracy != LINING_ACCURACY_PRECISE) {
			CodeCountsMap map = it->expand_code_counts_map();
			it->calculate_lining( map, lining_count_);
//			it->code_counts_map = map;
//			it->calculate_lining(lining_count_);
		}
		else {
/*			gint16 value = lining_count_;
			if (it->code == '6')
				value += 2000;
			if (it->code == '8')
				value += 6000;
*/
			it->calculate_lining(lining_count_);
		}
	}

	std::ofstream file;
	file.open( "lining.csv");
	for ( AssemblyIter it = assembly_.begin(); it != assembly_.end(); ++it) {
		for ( CodeCountsMap::iterator iter = it->code_counts_map.begin();
			iter != it->code_counts_map.end(); ++iter) {
			file << it->code << " " << int(iter->first);
			for ( Image::DataVector::iterator i = iter->second.begin();
					i != iter->second.end(); ++i) {
				
				file << " " << *i;
			}
			file << std::endl;
		}
	}
	file.close();
}

void
Data::set_chip_lining( char code, const std::vector<guint8>& lining)
{
	for ( AssemblyIter it = assembly_.begin(); it != assembly_.end(); ++it) {
		if (it->code == code)
			it->lining = lining;
	}
}

void
Data::set_lining(const std::map< char, std::vector<guint8> >& lining)
{
	for ( AssemblyIter it = assembly_.begin(); it != assembly_.end(); ++it)
		it->lining = lining.at(it->code);
}

} // namespace Scanner

} // namespace ScanAmati
