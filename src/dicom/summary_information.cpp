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

#include "patient_age.hpp"

/* files from src directory begin */
#include "global_strings.hpp"
#include "scanner/defines.hpp"
#include "scanner/x-ray.hpp"
#include "scanner/state.hpp"
/* files from src directory end */

#include <config.h>

#include <iostream>
#include "summary_information.hpp"

struct DefaultTagString {
	DcmTag tag;
	const char* value;
};

namespace {

typedef ScanAmati::DICOM::OrientationPositionPair Pair;
typedef ScanAmati::DICOM::PositionType PositionType;
typedef ScanAmati::DICOM::OrientationType OrientationType;

const PositionType SUPINE = ScanAmati::DICOM::POSITION_SUPINE;
const PositionType PRONE = ScanAmati::DICOM::POSITION_PRONE;
const PositionType LEFT = ScanAmati::DICOM::POSITION_DECUBITUS_LEFT;
const PositionType RIGHT = ScanAmati::DICOM::POSITION_DECUBITUS_RIGHT;

const OrientationType FEET = ScanAmati::DICOM::ORIENTATION_FEET_FIRST;
const OrientationType HEAD = ScanAmati::DICOM::ORIENTATION_HEAD_FIRST;

struct DefaultOrientationPosition {
	Pair pair;
	const char* value;
};

const DefaultOrientationPosition patient_position[] = {
	{ Pair( HEAD, SUPINE), "HFS" },
	{ Pair( HEAD, PRONE), "HFP" },
	{ Pair( HEAD, LEFT), "HFDL" },
	{ Pair( HEAD, RIGHT), "HFDR" },
	{ Pair( FEET, SUPINE), "FFS" },
	{ Pair( FEET, PRONE), "FFP" },
	{ Pair( FEET, LEFT), "FFDL" },
	{ Pair( FEET, RIGHT), "FFDR" },
	{ } // Terminating entry
};

const DefaultOrientationPosition patient_orientation[] = {
	{ Pair( HEAD, SUPINE), "L\\F" },
	{ Pair( HEAD, PRONE), "R\\F" },
	{ Pair( HEAD, LEFT), "L\\F" },
	{ Pair( HEAD, RIGHT), "L\\F" },
	{ Pair( FEET, SUPINE), "R\\H" },
	{ Pair( FEET, PRONE), "L\\H" },
	{ Pair( FEET, LEFT), "R\\H" },
	{ Pair( FEET, RIGHT), "R\\H" },
	{ } // Terminating entry
};

const DefaultOrientationPosition image_orientation_patient[] = {
	{ Pair( HEAD, SUPINE), "+1\\0\\0\\0\\0\\-1" },
	{ Pair( HEAD, PRONE), "-1\\0\\0\\0\\0\\-1" },
	{ Pair( HEAD, LEFT), "+1\\0\\0\\0\\0\\-1" },
	{ Pair( HEAD, RIGHT), "+1\\0\\0\\0\\0\\-1" },
	{ Pair( FEET, SUPINE), "-1\\0\\0\\0\\0\\+1" },
	{ Pair( FEET, PRONE), "+1\\0\\0\\0\\0\\+1" },
	{ Pair( FEET, LEFT), "-1\\0\\0\\0\\0\\+1" },
	{ Pair( FEET, RIGHT), "-1\\0\\0\\0\\0\\+1" },
	{ } // Terminating entry
};

struct DefaultOrientationPositionTag {
	const DefaultOrientationPosition* orientation_position;
	DcmTag tag;
} defaults[] = {
	{ patient_position, DCM_PatientPosition },
	{ patient_orientation, DCM_PatientOrientation },
	{ image_orientation_patient, DCM_ImageOrientationPatient },
	{ } // Terminating Entry
};

const char* dicom_unknown_id = "unknown";
const char* dicom_unknown_string = N_("Unknown");
const char* dicom_unknown_date_string = "19000101";
const char* dicom_unknown_time_string = "000000";
const char* dicom_unknown_age_string = "000Y";
const char* dicom_unknown_value_string = "0";
const char* dicom_unknown_sex_string = "O";
const char* dicom_manufacturer_string = N_("IHEP");

const char* pixel_spacing = "2.0000e-01\\2.0000e-01";

const DefaultTagString patient_defaults[] = {
	{ DCM_PatientName, gettext(dicom_unknown_string) },
	{ DCM_PatientID, dicom_unknown_id },
	{ DCM_PatientSex, dicom_unknown_sex_string },
	{ DCM_PatientWeight, dicom_unknown_value_string },
	{ DCM_PatientSize, dicom_unknown_value_string },
	{ DCM_PatientAddress, dicom_unknown_string },
	{ DCM_PatientAge, dicom_unknown_age_string },
	{ DCM_PatientBirthDate, dicom_unknown_date_string },
	{ DCM_PatientTelephoneNumbers, dicom_unknown_string },
	{ DCM_PatientComments, dicom_unknown_string },
	{ } // Terminating entry
};

const DefaultTagString study_defaults[] = {
	{ DCM_ReferringPhysicianName, gettext(dicom_unknown_string) },
	{ DCM_OperatorsName, gettext(dicom_unknown_string) },
	{ DCM_StudyID, dicom_unknown_id },
	{ DCM_BodyPartExamined, "UNKNOWN" },
	{ DCM_InstitutionName, dicom_unknown_string },
	{ DCM_InstitutionAddress, dicom_unknown_string },
	{ DCM_Manufacturer, gettext(dicom_manufacturer_string) },
	{ DCM_ManufacturerModelName, gettext(ScanAmati::Scanner::id_template) },
	{ DCM_DeviceSerialNumber, dicom_unknown_string },
	{ DCM_StudyDate, dicom_unknown_date_string },
	{ DCM_SeriesDate, dicom_unknown_date_string },
	{ DCM_AcquisitionDate, dicom_unknown_date_string },
	{ DCM_StudyTime, dicom_unknown_time_string },
	{ DCM_SeriesTime, dicom_unknown_time_string },
	{ DCM_AcquisitionTime, dicom_unknown_time_string },
	{ DCM_KVP, dicom_unknown_value_string },
	{ DCM_ExposureTime, dicom_unknown_value_string },
	{ DCM_Exposure, dicom_unknown_value_string },
	{ DCM_ExposureInuAs, dicom_unknown_value_string },
	{ DCM_XRayTubeCurrent, dicom_unknown_value_string },
	{ DCM_StudyDescription, dicom_unknown_string },
	{ DCM_ImageComments, dicom_unknown_string },
	{ DCM_ProtocolName, dicom_unknown_string },
	{ DCM_AdmittingDiagnosesDescription, dicom_unknown_string },
	{ DCM_SeriesDescription, dicom_unknown_string },
	{ } // Terminating entry
};

const DefaultTagString file_defaults[] = {
	{ DCM_SOPClassUID, UID_DigitalXRayImageStorageForPresentation },
	{ DCM_TypeOfPatientID, "TEXT" },
	{ DCM_Modality, "DX" },
	{ DCM_ImageType, "ORIGINAL\\PRIMARY" },
	{ DCM_ImageLaterality, "R" },
	{ DCM_SpecificCharacterSet, "ISO_IR 192" },
	{ DCM_ImagerPixelSpacing, pixel_spacing },
	{ DCM_PixelSpacing, pixel_spacing },
	{ DCM_InstanceNumber, "1" },
	{ DCM_SoftwareVersions, PACKAGE_STRING },
	{ DCM_SeriesNumber, "1" },
	{ DCM_PresentationIntentType, "FOR PRESENTATION" },
	{ DCM_SamplesPerPixel, "1" },
	{ DCM_PhotometricInterpretation, "MONOCHROME2" },
	{ DCM_BitsAllocated, "16" },
#if SCANNER_ADC_RESOLUTION == 14
	{ DCM_BitsStored, "14" },
	{ DCM_HighBit, "13" },
#elif SCANNER_ADC_RESOLUTION == 12
	{ DCM_BitsStored, "12" },
	{ DCM_HighBit, "11" },
#endif
	{ DCM_PixelRepresentation, "0000H" },
	{ DCM_PixelIntensityRelationship, "LIN" },
	{ DCM_PixelIntensityRelationshipSign, "1" },
	{ DCM_RescaleIntercept, "0" },
	{ DCM_RescaleSlope, "1" },
	{ DCM_RescaleType, "US" },
	{ DCM_PresentationLUTShape, "IDENTITY" },
	{ DCM_LossyImageCompression, "00" },
	{ DCM_BurnedInAnnotation, "NO" },
#if SCANNER_ADC_RESOLUTION == 14
	{ DCM_WindowCenter, "8193" },
	{ DCM_WindowWidth, "16384" },
#elif SCANNER_ADC_RESOLUTION == 12
	{ DCM_WindowCenter, "2047" },
	{ DCM_WindowWidth, "4095" },
#endif
	{ DCM_ScanOptions, "STEP" },
	{ DCM_DetectorType, "DIRECT" },
	{ DCM_DetectorConfiguration, "SLOT" },
	{ DCM_DetectorActiveShape, "RECTANGLE" },
	{ } // Terminating Entry
};

const DefaultTagString scanner_defaults[] = {
	{ DCM_KVP, dicom_unknown_value_string }, // kV
	{ DCM_Exposure, dicom_unknown_value_string }, // mAs
	{ DCM_ExposureInuAs, dicom_unknown_value_string }, // uAs
	{ DCM_ExposureTime, dicom_unknown_value_string }, // ms
	{ DCM_XRayTubeCurrent, dicom_unknown_value_string }, // mA
	{ DCM_DetectorTemperature, dicom_unknown_value_string }, // Celsius
	{ } // Terminating Entry
};

const DefaultTagString private_defaults[] = {
	{ PrivateCreatorTag, dicom_manufacturer_string },
	{ PrivateEffectiveDoseTag, dicom_unknown_value_string },
	{ PrivateBodyPartTag, dicom_unknown_string },
	{ } // Terminating Entry
};

const DefaultTagString uid_tags[] = {
	{ DCM_SOPInstanceUID, SITE_INSTANCE_UID_ROOT },
	{ DCM_StudyInstanceUID, SITE_STUDY_UID_ROOT },
	{ DCM_SeriesInstanceUID, SITE_SERIES_UID_ROOT },
	{ } // Terminating Entry
};

DefaultTagString*
create_uid_defaults()
{
	int i = 0;
	while (uid_tags[i].value) {
		++i;
	}

	DefaultTagString* uids = new DefaultTagString[i + 1]; // + Terminating Entry
	for ( int j = 0; j < i; ++j) {
		char* value = new char[100];
		uids[j].tag = uid_tags[j].tag;
		uids[j].value = dcmGenerateUniqueIdentifier( value, uid_tags[j].value);
	}
	uids[i].tag = DcmTag();
	uids[i].value = 0;

	return uids;
}

void
delete_uid_defaults(DefaultTagString* defs)
{
	int i = 0;
	while (defs[i].value) {
		delete [] defs[i].value;
		++i;
	};
	delete [] defs;
}

} // namespace

namespace ScanAmati {

namespace DICOM {

TagStringMap::TagStringMap(DcmDataset* dataset)
{
	load( dataset, file_defaults);
	load( dataset, private_defaults);

	DefaultTagString* uids = create_uid_defaults();
	load( dataset, uids);
	delete_uid_defaults(uids);
}

bool
TagStringMap::load( DcmDataset* set, const DefaultTagString* defs)
{
	int i = 0;
	while (defs[i].value) {
		const char* str = (defs[i].tag == DCM_PatientSex) ? defs[i].value :
			gettext(defs[i].value);
		map_.insert(DcmTagStringPair( defs[i].tag, Glib::ustring(str)));

		if (set) {
			const char* string = 0;
			if (set->findAndGetString( defs[i].tag, string).good())
				map_[defs[i].tag] = (string) ? Glib::ustring(string) : Glib::ustring();
		}
		++i;
	}
	return true;
}

bool
PatientInfo::load(DcmDataset* set)
{
	return TagStringMap::load( set, patient_defaults);
}

bool
StudyInfo::load(DcmDataset* set)
{
	return TagStringMap::load( set, study_defaults);
}

SummaryInfo::SummaryInfo(DcmDataset* set)
	:
	PatientInfo(set),
	StudyInfo(set)
{
	TagStringMap::load( set, scanner_defaults);
}

void
SummaryInfo::save(DcmDataset* dataset)
{
	TagStringMap::save(dataset);
	save_private_elements(dataset);
}

bool
SummaryInfo::set_position_orientation(
	const OrientationPositionPair& pair)
{
	int i = 0;
	while (defaults[i].orientation_position) {
		int j = 0;
		while (defaults[i].orientation_position[j].value) {
			const char* str = 0;
			if (defaults[i].orientation_position[j].pair == pair) {
				str = defaults[i].orientation_position[j].value;
				if (str) {
					StudyInfo::value(defaults[i].tag) = Glib::ustring(str);
					break;
				}
			}
			++j;
		}
		++i;
	}

	return false;
}

bool
SummaryInfo::get_position_orientation(OrientationPositionPair& pair)
{
	return true;
}

Glib::ustring
SummaryInfo::get_patient_sex(SexStringType type)
{

	Glib::ustring str = PatientInfo::sex();
	SexType sex = sex_type(str);

	return format_person_sex( sex, type);
}

void
SummaryInfo::save_private_elements(DcmDataset* set)
{
	if (!set->tagExists(private_defaults[0].tag)) {
		int i = 0;
		while (private_defaults[i].value) {
			set->putAndInsertString( private_defaults[i].tag,
				PatientInfo::value(private_defaults[i].tag).c_str());
			++i;
		}
	}
}

void
SummaryInfo::set_xray_parameters(const XrayParameters& params)
{
	StudyInfo::set_DS( DCM_KVP, params.voltage);
	StudyInfo::set_IS( DCM_Exposure,
		static_cast<int>(params.exposure));
	StudyInfo::set_IS( DCM_ExposureInuAs,
		static_cast<int>(params.exposure * 1000));
	StudyInfo::set_IS( DCM_ExposureTime,
		static_cast<int>(params.exposure_time() * 1000));
	StudyInfo::set_IS( DCM_XRayTubeCurrent,
		static_cast<int>(params.current()));
}

void
SummaryInfo::set_scanner_state(const Scanner::State& state)
{
	StudyInfo::set_DS( DCM_DetectorTemperature, state.temperature());
	StudyInfo::set_LO( DCM_ManufacturerModelName, state.id());
}

void
SummaryInfo::set_acquisition_current_date_time()
{
	Glib::Date date;
	OFTime time;

	date.set_time_current();
	time.setCurrentTime();

	StudyInfo::Info::set_date( DCM_AcquisitionDate, date);
	StudyInfo::Info::set_TM( DCM_AcquisitionTime, time);
}

Glib::ustring
SummaryInfo::get_tooltip_text()
{
	const Glib::ustring blank(_("Name: %1\nStudy: %2\nDate: %3\nTime: %4"));

	Glib::ustring name = Glib::ustring::compose( "%1 (%2)",
		PatientInfo::get_name(), PatientInfo::get_age());

	return Glib::ustring::compose( blank, name, StudyInfo::get_id(),
		StudyInfo::get_date().format_string("%x"),
		DICOM::format_oftime(StudyInfo::get_time()));
}

Glib::ustring
SummaryInfo::get_label_text()
{
	return PatientInfo::get_name();
}

} // namespace DICOM

} // namespace ScanAmati
