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
 
#include "operation.hpp"

/* files from src directory begin */
#include "dicom/patient_age.hpp"
#include "dicom/utils.hpp"
#include "dicom/summary_information.hpp"
/* files from src directory end */

#define OFFSET 5.

namespace ScanAmati {

namespace Print {

Operation::Operation( const DICOM::SummaryInfo& info, double title_font_size,
	double subtitle_font_size, double default_font_size)
	:
	dicom_info_(info),
	title_font_size_(title_font_size),
	subtitle_font_size_(subtitle_font_size),
	default_font_size_(default_font_size),
	width_(0),
	height_(0)
{
}

Operation::~Operation()
{
}

Glib::RefPtr<Operation>
Operation::create( const DICOM::SummaryInfo& info, double title_font_size,
	double subtitle_font_size, double default_font_size)
{
	return Glib::RefPtr<Operation>(new Operation( info, title_font_size,
		subtitle_font_size, default_font_size));
}

void
Operation::on_begin_print(const Glib::RefPtr<Gtk::PrintContext>& print)
{
	width_ = print->get_width();
	height_ = print->get_height();

	//Get a Cairo Context, which is used as a drawing board:
	Cairo::RefPtr<Cairo::Context> cairo = print->get_cairo_context();

	cairo->select_font_face( "Sans", Cairo::FONT_SLANT_NORMAL,
		Cairo::FONT_WEIGHT_NORMAL);

	//We'll use black letters:
	cairo->set_source_rgb( 0, 0, 0);

	draw_title(print);
	draw_patient_info(print);
	draw_study_info(print);
}

void
Operation::on_draw_page( const Glib::RefPtr<Gtk::PrintContext>&, int)
{
}

void
Operation::draw_title(const Glib::RefPtr<Gtk::PrintContext>& print)
{
	double pos_x, pos_y;

	Cairo::RefPtr<Cairo::Context> cairo = print->get_cairo_context();

	DICOM::SummaryInfo* info = const_cast<DICOM::SummaryInfo*>(&dicom_info_);
	DICOM::StudyInfo* study = info->get_study_info();

	// Show Institution Name
	cairo->set_font_size(title_font_size_);
	Cairo::TextExtents cur_ext, prev_ext;
	cairo->get_text_extents( study->get_institution_name(), cur_ext);

	pos_x = (width_ - cur_ext.width) / 2.;
	pos_y = cur_ext.height + OFFSET;
	cairo->move_to( pos_x, pos_y);
	cairo->show_text(study->get_institution_name());

	// Show Institution Address
	prev_ext = cur_ext;
	cairo->set_font_size(subtitle_font_size_);
	cairo->get_text_extents( study->get_institution_address(), cur_ext);

	pos_x = -(prev_ext.width + prev_ext.x_bearing +
		cur_ext.width + cur_ext.x_bearing) / 2.;
	pos_y = cur_ext.height + OFFSET;
	cairo->rel_move_to( pos_x, pos_y);
	cairo->show_text(study->get_institution_address());

	// Show Study Description
	prev_ext = cur_ext;
	cairo->set_font_size(title_font_size_);
	cairo->get_text_extents( study->get_description(), cur_ext);

	pos_x = -(prev_ext.width + prev_ext.x_bearing +
		cur_ext.width + cur_ext.x_bearing) / 2.;
	pos_y = cur_ext.height + OFFSET;
	cairo->rel_move_to( pos_x, pos_y);
	cairo->show_text(study->get_description());


	pos_x = -(width_ + cur_ext.width) / 2.;
	pos_y = cur_ext.height + OFFSET;
	cairo->rel_move_to( pos_x, pos_y);

	cairo->stroke_preserve();
}

void
Operation::draw_study_info(const Glib::RefPtr<Gtk::PrintContext>& print)
{ 
	Cairo::RefPtr<Cairo::Context> cairo = print->get_cairo_context();

	Glib::RefPtr<Pango::Layout> layout = print->create_pango_layout();

	Pango::FontDescription font_desc("sans 12");
	layout->set_font_description(font_desc);

	layout->set_width(width_ * Pango::SCALE);

	DICOM::SummaryInfo* info = const_cast<DICOM::SummaryInfo*>(&dicom_info_);
	DICOM::StudyInfo* study = info->get_study_info();

	//Set and mark up the text to print:
	Glib::ustring marked_up_form_text = "<b>Study ID</b>: " + study->get_id() + "\n";

	layout->set_markup(marked_up_form_text);
	layout->show_in_cairo_context(cairo);
	cairo->stroke();

	Glib::ustring txt = DICOM::format_person_name(study->get_operator_name());
	marked_up_form_text = "<b>Operator's name</b>: " + txt + "\t\t\t\n";

	txt = DICOM::format_person_name(study->get_physician_name());
	marked_up_form_text += "<b>Physician's name</b>: " + txt + "\t\t\t\n";

	layout->set_markup(marked_up_form_text);

	int w, h;
	layout->get_pixel_size( w, h);

	cairo->move_to( width_ - w, height_ - h);
	layout->show_in_cairo_context(cairo);

	cairo->stroke();
}

void
Operation::draw_patient_info(const Glib::RefPtr<Gtk::PrintContext>& print)
{
	Cairo::RefPtr<Cairo::Context> cairo = print->get_cairo_context();

	Glib::RefPtr<Pango::Layout> layout = print->create_pango_layout();

	Pango::FontDescription font_desc("sans 12");
	layout->set_font_description(font_desc);

	layout->set_width(width_ * Pango::SCALE);

	DICOM::SummaryInfo* info = const_cast<DICOM::SummaryInfo*>(&dicom_info_);
	DICOM::PatientInfo* patient = info->get_patient_info();

	//Set and mark up the text to print:
	Glib::ustring marked_up_form_text;
	Glib::ustring txt = DICOM::format_person_name(patient->get_name());
	marked_up_form_text += "<b>Patient's name</b>: " + txt + "\n";
	marked_up_form_text += "<b>Sex</b>: " + info->get_patient_sex() + "\n";

	DICOM::PatientAge age = info->get_patient_age();
	marked_up_form_text += "<b>Age</b>: " + age.age_string() + "\n\n";

	layout->set_markup(marked_up_form_text);
	layout->show_in_cairo_context(cairo);

	int w, h;
	layout->get_pixel_size( w, h);
	cairo->rel_move_to( 0, h);
	cairo->stroke_preserve();
}

} // namespace Print

} // namespace ScanAmati
