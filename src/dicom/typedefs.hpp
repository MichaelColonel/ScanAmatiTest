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
#include <vector>
#include <glibmm/ustring.h>

// files from src directory begin
#include "dcmtk_defines.hpp"
// files from src directory end

namespace ScanAmati {

namespace DICOM {

enum IntensityType {
	INTENSITY_LINEAR,
	INTENSITY_LOGARITHMIC
};

enum IntentType {
	FOR_PROCESSING,
	FOR_PRESENTATION
};

enum PositionType { // all positions are coronal
	POSITION_SUPINE, // on the back, PZ
	POSITION_PRONE, // on the face, ZP
	POSITION_DECUBITUS_LEFT, // on the left side, B
	POSITION_DECUBITUS_RIGHT // on the right side, B
};

enum OrientationType {
	ORIENTATION_HEAD_FIRST,
	ORIENTATION_FEET_FIRST
};

enum AgeType {
	AGE_YEARS,
	AGE_MONTHS,
	AGE_WEEKS,
	AGE_DAYS
};

enum SexType {
	SEX_OTHER,
	SEX_FEMALE,
	SEX_MALE
};

enum SexStringType {
	SEX_STRING_LONG,
	SEX_STRING_SHORT
};

typedef std::vector<PositionType> PositionVector;
typedef std::map< PositionType, Glib::ustring> PositionStringMap;
typedef std::pair< PositionType, Glib::ustring> PositionStringPair;

typedef std::vector<OrientationType> OrientationVector;
typedef std::map< OrientationType, Glib::ustring> OrientationStringMap;
typedef std::pair< OrientationType, Glib::ustring> OrientationStringPair;

typedef std::pair< OrientationType, PositionType> OrientationPositionPair;
typedef std::map< OrientationPositionPair, Glib::ustring>
	OrientationPositionStringMap;
typedef std::pair< OrientationPositionPair, Glib::ustring>
	OrientationPositionStringPair;

typedef std::map< DcmTag, Glib::ustring> DcmTagStringMap;
typedef std::pair< DcmTag, Glib::ustring> DcmTagStringPair;

} // namespace DICOM

} // namespace ScanAmati
