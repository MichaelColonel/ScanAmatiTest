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

#include <gtkmm/adjustment.h>
#include <gtkmm/frame.h>
#include <gtkmm/scrollbar.h>

#include "image_area.hpp"
#include "palette_area.hpp"
#include "scrolled_image_area.hpp"

namespace ScanAmati {

namespace UI {

ScrolledImageArea::ScrolledImageArea( BaseObjectType* cobject,
	const Glib::RefPtr<Gtk::Builder>& builder)
	:
	Gtk::Table(cobject),
	builder_(builder),
	scrollbar_width_(0),
	scrollbar_height_(0),
	image_area_(0),
	palette_area_(0)
{
	init_ui();
	connect_signals();
}

ScrolledImageArea::~ScrolledImageArea()
{
}

void
ScrolledImageArea::init_ui()
{
	// show horizontal scrollbar
	builder_->get_widget( "hscrollbar-image-width", scrollbar_width_);
	
	// show vertical scrollbar
	builder_->get_widget( "vscrollbar-image-height", scrollbar_height_);

	// show image area
	builder_->get_widget_derived( "drawingarea-image", image_area_);

	// show palette area
	builder_->get_widget_derived( "drawingarea-palette", palette_area_);
}

void
ScrolledImageArea::connect_signals()
{	
	signal_scroll_position_changed_.connect(sigc::mem_fun(
		image_area_, &ImageArea::set_scroll_to));

	image_area_->signal_scale_changed().connect(sigc::hide(sigc::mem_fun(
		*this, &ScrolledImageArea::on_scale_changed)));
	image_area_->signal_size_allocate().connect(sigc::mem_fun(
		*this, &ScrolledImageArea::on_image_area_allocation_changed));

	scrollbar_width_->signal_value_changed().connect(sigc::mem_fun(
		*this, &ScrolledImageArea::on_adjustment_value_changed));
	scrollbar_height_->signal_value_changed().connect(sigc::mem_fun(
		*this, &ScrolledImageArea::on_adjustment_value_changed));
}

void
ScrolledImageArea::on_adjustment_value_changed()
{
	const Gtk::Adjustment* adj;

	adj = scrollbar_width_->get_adjustment();
	int x = static_cast<int>(adj->get_value());

	adj = scrollbar_height_->get_adjustment();
	int y = static_cast<int>(adj->get_value());

	signal_scroll_position_changed_( x, y);
}

void
ScrolledImageArea::on_scale_changed()
{
	int w, h;
	image_area_->get_image_size( w, h);

	Gtk::Allocation wa = image_area_->get_allocation();
	Gtk::Adjustment *adj;

	adj = scrollbar_width_->get_adjustment();
	adj->set_value(0);
	adj->set_lower(0);
	adj->set_upper(w);
	adj->set_page_increment(wa.get_width());
	adj->set_step_increment(0.1 * wa.get_width());
	adj->set_page_size(wa.get_width());
	int x = static_cast<int>(adj->get_value());

	adj = scrollbar_height_->get_adjustment();
	adj->set_value(0);
	adj->set_lower(0);
	adj->set_upper(h);
	adj->set_page_increment(wa.get_height());
	adj->set_step_increment(0.1 * wa.get_height());
	adj->set_page_size(wa.get_height());
	int y = static_cast<int>(adj->get_value());

	signal_scroll_position_changed_( x, y);
}

void
ScrolledImageArea::on_image_area_allocation_changed(Gtk::Allocation& wa)
{
	Gtk::Adjustment *adj;

	adj = scrollbar_width_->get_adjustment();
	if ((wa.get_width() + adj->get_value() > adj->get_upper()) &&
		(wa.get_width() < adj->get_upper())) {
		double diff = adj->get_upper() - wa.get_width();
		double value = adj->get_value();
		adj->set_value(diff);
		adj->set_page_size(adj->get_upper() - diff);
	}
	else
		adj->set_page_size(wa.get_width());

	adj = scrollbar_height_->get_adjustment();
	if ((wa.get_height() + adj->get_value() > adj->get_upper()) &&
		(wa.get_height() < adj->get_upper())) {
		double diff = adj->get_upper() - wa.get_height();
		double value = adj->get_value();
		adj->set_value(diff);
		adj->set_page_size(adj->get_upper() - diff);
	}
	else
		adj->set_page_size(wa.get_height());
}

void
ScrolledImageArea::clear_area()
{
	Gtk::Adjustment *adj;

	adj = scrollbar_width_->get_adjustment();
	adj->set_value(0.0);
	adj->set_lower(0.0);
	adj->set_upper(100.0);
	adj->set_page_increment(0.0);
	adj->set_step_increment(0.0);
	adj->set_page_size(100.0);

	adj = scrollbar_height_->get_adjustment();
	adj->set_value(0);
	adj->set_lower(0);
	adj->set_upper(100.0);
	adj->set_page_increment(0.0);
	adj->set_step_increment(0.0);
	adj->set_page_size(100.0);

	image_area_->clear_area();
}

} // namepace UI

} //namespace ScanAmati
