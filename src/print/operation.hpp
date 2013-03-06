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

#include <gtkmm/printoperation.h>
#include <cairomm/context.h>
#include <vector>

namespace ScanAmati {

namespace DICOM {
class SummaryInfo;
} // namespace DICOM

namespace Print {

class Operation : public Gtk::PrintOperation {
public:
	static Glib::RefPtr<Operation> create( const DICOM::SummaryInfo&,
		double title_font_size = 28., double subtitle_font_size = 18.,
		double default_font_size = 12.);
	virtual ~Operation();

protected:
	Operation( const DICOM::SummaryInfo&,
		double title_font_size,
		double subtitle_font_size,
		double default_font_size);

	//PrintOperation default signal handler overrides:
	virtual void on_begin_print(const Glib::RefPtr<Gtk::PrintContext>&);
	virtual void on_draw_page( const Glib::RefPtr<Gtk::PrintContext>&, int);

private:
	void draw_title(const Glib::RefPtr<Gtk::PrintContext>&);
	void draw_patient_info(const Glib::RefPtr<Gtk::PrintContext>&);
	void draw_study_info(const Glib::RefPtr<Gtk::PrintContext>&);

	const DICOM::SummaryInfo& dicom_info_;
	double title_font_size_;
	double subtitle_font_size_;
	double default_font_size_;
	double width_;
	double height_;
};

} // namespace Print

} // namespace ScanAmati
