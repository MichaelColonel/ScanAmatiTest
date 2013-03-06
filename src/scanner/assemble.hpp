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

#include <map>

#include "defines.hpp"

/* files from src directory begin */
#include "image/summary_data.hpp"
/* files from src directory begin */

namespace ScanAmati {
	
namespace Scanner {

typedef std::map< guint8, Image::DataVector> CodeCountsMap;
typedef CodeCountsMap::value_type CodeCountsPair;

struct Assemble {
	Assemble() : code('0'), lining(SCANNER_STRIPS_PER_CHIP_REAL) {}
	~Assemble() {}
	void clear_for_lining_acquisition() { code_counts_map.clear(); }
	void clear_for_image_acquisition();
	void clear_after_disconnect();
	void calculate_lining(gint16 count);
	void calculate_lining( const CodeCountsMap& map, gint16 count);
	CodeCountsMap expand_code_counts_map() const;

	char code; // do not delete or clear
	Image::DataVector pedestals;
	Image::DataSharedPtr raw_data;
	std::vector<guint8> lining; // do not delete or clear
	std::vector<guint> bad_strips;
	CodeCountsMap code_counts_map;
};

typedef std::vector<Assemble> AssemblyVector;
typedef AssemblyVector::iterator AssemblyIter;
typedef AssemblyVector::const_iterator AssemblyConstIter;

} // namespace Scanner

} // namespace ScanAmati
