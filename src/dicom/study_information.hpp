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

namespace ScanAmati {

namespace DICOM {

class StudyInfo : virtual public Info {
public:
	virtual ~StudyInfo() {}
	Glib::Date get_date() { return Info::get_date(DCM_StudyDate); }
	Glib::Date get_date(const DcmTagKey& key) { return Info::get_date(key); }
	bool set_date(const Glib::Date&);
	OFTime get_time() { return get_TM(DCM_StudyTime); }
	OFTime get_time(const DcmTagKey& key) { return get_TM(key); }
	bool set_time(const OFTime&);
	Glib::ustring get_operator_name();
	bool set_operator_name(const Glib::ustring& name);
	Glib::ustring get_physician_name();
	bool set_physician_name(const Glib::ustring& name);
	Glib::ustring get_manufacturer() { return Info::value(DCM_Manufacturer); }
	Glib::ustring get_institution_name() { return Info::value(DCM_InstitutionName); }
	void set_institution_name(const Glib::ustring& name) { value(DCM_InstitutionName) = name; }
	Glib::ustring get_institution_address() { return Info::value(DCM_InstitutionAddress); }
	void set_institution_address(const Glib::ustring& address) { value(DCM_InstitutionAddress) = address; }
	Glib::ustring get_id() { return Info::value(DCM_StudyID); }
	bool set_id(const Glib::ustring& id) { return Info::set_SH( DCM_StudyID, id); }
	Glib::ustring get_description() { return Info::value(DCM_StudyDescription); }
	bool set_description(const Glib::ustring& descr) { return Info::set_LO( DCM_StudyDescription, descr); }
	Glib::ustring get_protocol_name() { return Info::get_LO(DCM_ProtocolName); }
	bool set_protocol_name(const Glib::ustring& protocol) { return Info::set_LO( DCM_ProtocolName, protocol); }
	void set_current_date_time();

protected:
	StudyInfo(DcmDataset* dataset) : Info(dataset) { this->load(dataset); }
	virtual bool load(DcmDataset* dataset);
};

inline
Glib::ustring
StudyInfo::get_operator_name()
{
	return Info::get_PN(DCM_OperatorsName);
}

inline
bool
StudyInfo::set_operator_name(const Glib::ustring& name)
{
	return Info::set_PN( DCM_OperatorsName, name);
}

inline
Glib::ustring
StudyInfo::get_physician_name()
{
	return Info::get_PN(DCM_ReferringPhysicianName);
}

inline
bool
StudyInfo::set_physician_name(const Glib::ustring& name)
{
	return Info::set_PN( DCM_ReferringPhysicianName, name);
}

inline
bool
StudyInfo::set_date(const Glib::Date& date)
{
	Info::set_date( DCM_StudyDate, date);
	Info::set_date( DCM_SeriesDate, date);
	return Info::set_date( DCM_AcquisitionDate, date);
}

inline
bool
StudyInfo::set_time(const OFTime& time)
{
	Info::set_TM( DCM_StudyTime, time);
	Info::set_TM( DCM_SeriesTime, time);
	return Info::set_TM( DCM_AcquisitionTime, time);
}

inline
void
StudyInfo::set_current_date_time()
{
	Glib::Date date;
	date.set_time_current();
	set_date(date);
	
	OFTime time;
	time.setCurrentTime();
	set_time(time);
}

} // namespace DICOM

} // namespace ScanAmati
