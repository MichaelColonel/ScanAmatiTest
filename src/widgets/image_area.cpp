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

#include <iostream>
#include <gdkmm/cursor.h>
#include <gtkmm/menu.h>

#include <cmath>

// files from src directory begin
#include "palette/palette.hpp"
#include "scanner/manager.hpp"
// files from src directory end

#include "image_area.hpp"

namespace {

const char* const chip_strings[SCANNER_CHIPS] = {
#if SCANNER_CHIPS == 16
	"0", "1", "2", "3",
	"4", "5", "6", "7",
	"8", "9", "A", "B",
	"C", "D", "E", "F"
#elif SCANNER_CHIPS == 12
	"2", "3", "4", "5",
	"6", "7", "8", "9",
	"A", "B", "C", "D"
#endif
};

Gdk::Color gray("gray");

void
destroy_pixbuf(const guint8* data)
{
	std::cout << "destroy" << std::endl;
	delete [] data;
	data = 0;
}

Gdk::Pixbuf::SlotDestroyData destroy_slot = sigc::ptr_fun(&destroy_pixbuf);

} // namespace

namespace ScanAmati {

namespace UI {

ImageArea::ImageArea( BaseObjectType* cobject,
	const Glib::RefPtr<Gtk::Builder>&)
	:
	Gtk::DrawingArea(cobject),
	pos_x_(0),
	pos_y_(0),
	image_width_(0),
	image_height_(0),
	width_(0),
	height_(0),
	vis_width_(0),
	vis_height_(0),
	x_offset_(0),
	y_offset_(0),
	scale_(1.0),
	zoom_(ZOOM_HEIGHT),
	margins_(false),
	palette_(0),
	menu_(0)
{
	Glib::RefPtr<Gdk::Colormap> colormap = get_default_colormap();

	colormap->alloc_color(gray);
}

ImageArea::~ImageArea()
{
}

bool
ImageArea::on_expose_event(GdkEventExpose* event)
{
	// This is where we draw on the window
	Glib::RefPtr<Gdk::Window> window = get_window();

	if (window) {
		if (pixmap_) {
			window->draw_drawable( get_style()->get_black_gc(),
				pixmap_,
				pos_x_,
				pos_y_,
				x_offset_,
				y_offset_,
				vis_width_,
				vis_height_);
		}
	}

	return false;
}

bool
ImageArea::on_motion_notify_event(GdkEventMotion* event)
{
	if (Glib::RefPtr<Gdk::Window> window = get_window()) {
		if (pointer_within_image(event))
			window->set_cursor(Gdk::Cursor(Gdk::CROSS));
		else
			window->set_cursor();
	}
	return false;
}

bool
ImageArea::on_enter_notify_event(GdkEventCrossing*)
{
	if (Glib::RefPtr<Gdk::Window> window = get_window()) {
		if (pixbuf_)
			window->set_cursor(Gdk::Cursor(Gdk::HAND2));
	}
	return false;
}

bool
ImageArea::on_leave_notify_event(GdkEventCrossing*)
{
	if (Glib::RefPtr<Gdk::Window> window = get_window()) {
		window->set_cursor();
	}
	return false;
}

bool
ImageArea::on_button_press_event(GdkEventButton* event)
{
	bool res = false;
	if ((event->type == GDK_BUTTON_PRESS) && (event->button == 3)) {
		menu_->popup( event->button, event->time);
		res = true;
	}
	return res;
}

void
ImageArea::on_zoom(ZoomType type)
{
	zoom_ = type;

	if (pixbuf_) {
		Gtk::Allocation wa = get_allocation();

		update_scale( wa.get_width(), wa.get_height());
		update_pixbuf();
		update_image_visible_margins( wa.get_width(), wa.get_height());
		update_pixmap();

		signal_scale_(scale_);
		get_window()->invalidate_rect( wa, false);
	}
}

void
ImageArea::update_image_visible_margins( int width, int height)
{
	vis_width_ = MIN( width, pixbuf_->get_width()); // image region width
	vis_height_ = MIN( height, pixbuf_->get_height()); // image region height

	x_offset_ = (width - vis_width_) / 2.;
	y_offset_ = (height - vis_height_) / 2.;
}

void
ImageArea::update_scale( int width, int height)
{
	switch (zoom_) {
	case ZOOM_HEIGHT:
		scale_ = double(height) / image_height_;
		break;
	case ZOOM_WIDTH:
		scale_ = double(width) / image_width_;
		break;
	case ZOOM_100:
		scale_ = 1.0;
		break;
	case ZOOM_200:
		scale_ = 2.0;
		break;
	case ZOOM_300:
		scale_ = 3.0;
		break;
	default:
		scale_ = 1.0;
		break;
	}
	width_ = scale_ * image_width_;
	height_ = scale_ * image_height_;
}

sigc::signal< void, double>
ImageArea::signal_scale_changed()
{
	return signal_scale_;
}

void
ImageArea::on_realize()
{
	modify_bg( Gtk::STATE_NORMAL, gray);

	// call base class:
	Gtk::DrawingArea::on_realize();
}

void
ImageArea::get_image_size( int& width, int& height) const
{
	switch (zoom_) {
	case ZOOM_HEIGHT:
	case ZOOM_WIDTH:
		width = width_;
		height = height_;
		break;
	case ZOOM_100:
		width = image_width_;
		height = image_height_;
		break;
	case ZOOM_200:
		width = 2 * image_width_;
		height = 2 * image_height_;
		break;
	case ZOOM_300:
		width = 3 * image_width_;
		height = 3 * image_height_;
		break;
	default:
		width = image_width_;
		height = image_height_;
		break;
	}
}

void
ImageArea::update_pixbuf()
{
	Glib::RefPtr<Gdk::Pixbuf> pixbuf = Gdk::Pixbuf::create_from_data(
		&buf_[0], Gdk::COLORSPACE_RGB, false, 8,
		image_width_, image_height_, 3 * image_width_);

	switch (zoom_) {
	case ZOOM_HEIGHT:
	case ZOOM_WIDTH:
	case ZOOM_200:
	case ZOOM_300:
		pixbuf_ = pixbuf->scale_simple( width_, height_, Gdk::INTERP_HYPER);
		break;
	case ZOOM_100:
	default:
		pixbuf_ = pixbuf;
		break;
	}
}

void
ImageArea::update_pixmap()
{
	int w, h;
	switch (zoom_) {
	case ZOOM_HEIGHT:
	case ZOOM_WIDTH:
	case ZOOM_200:
	case ZOOM_300:
		w = width_;
		h = height_;
		break;
	case ZOOM_100:
	default:
		w = image_width_;
		h = image_height_;
		break;
	}

	pixmap_ = Gdk::Pixmap::create( get_window(), w, h, -1);
	pixmap_->draw_pixbuf( get_style()->get_black_gc(), pixbuf_,
		0, 0, 0, 0, w, h, Gdk::RGB_DITHER_NONE, 0, 0);

	if (margins_) {
		Cairo::RefPtr<Cairo::Context> cr = pixmap_->create_cairo_context();
		cr->set_source_rgb( .337, .612, .117); // green
		cr->rectangle( 0, 0, w, h);
		cr->clip();
		cr->save();

		draw_margins( cr, w, h);
		draw_numbers(cr);

		cr->restore();
		cr->stroke();
	}
	if (!broken_strips_.empty()) {
		Cairo::RefPtr<Cairo::Context> cr = pixmap_->create_cairo_context();
		cr->set_source_rgb( .9, .1, .1); // red
		cr->rectangle( 0, 0, w, h);
		cr->clip();
		cr->save();

		draw_broken_strips( cr, h);
		cr->restore();
		cr->stroke();
	}
}

bool
ImageArea::on_configure_event(GdkEventConfigure* event)
{
	if (pixbuf_) {
		update_image_visible_margins( event->width, event->height);
		update_pixmap();
	}
	return true;
}

void
ImageArea::set_scroll_to( int x, int y)
{
	pos_x_ = x;
	pos_y_ = y;
	queue_draw();
}

void
ImageArea::set_palette(const Image::Palette* palette)
{
	palette_ = palette;

	if (pixbuf_) {

		for ( int i = 0; i < image_height_ * image_width_; i++) {
			guint8 pix = buf_color_[i];
			if (palette_) {
				buf_[3 * i + 0] = palette_->rgb(3 * pix + 0);
				buf_[3 * i + 1] = palette_->rgb(3 * pix + 1);
				buf_[3 * i + 2] = palette_->rgb(3 * pix + 2);
			}
			else
				buf_[3 * i + 0] = buf_[3 * i + 1] = buf_[3 * i + 2] = pix;
		}

		Gtk::Allocation wa = get_allocation();

		update_scale( wa.get_width(), wa.get_height());
		update_pixbuf();
		update_image_visible_margins( wa.get_width(), wa.get_height());
		update_pixmap();

		queue_draw();
	}
}

void
ImageArea::set_image_data(const Image::SummaryData& data)
{
	buf_color_ = data.image_buffer();
	const Image::DataSharedPtr& image = data.raw_data();

	if (image) {
		image_height_ = image->height();
		image_width_ = image->width();

		update_image_data();
	}
	else {
		clear_area();
	}
}

void
ImageArea::update_image_data()
{
	buf_.resize(3 * image_height_ * image_width_);

	for ( int i = 0; i < image_height_ * image_width_; i++) {
		guint8 pix = buf_color_[i];
		if (palette_) {
			buf_[3 * i + 0] = palette_->rgb(3 * pix + 0);
			buf_[3 * i + 1] = palette_->rgb(3 * pix + 1);
			buf_[3 * i + 2] = palette_->rgb(3 * pix + 2);
		}
		else
			buf_[3 * i + 0] = buf_[3 * i + 1] = buf_[3 * i + 2] = pix;
	}

	Gtk::Allocation wa = get_allocation();

	update_scale( wa.get_width(), wa.get_height());
	update_pixbuf();
	update_image_visible_margins( wa.get_width(), wa.get_height());
	update_pixmap();

	signal_scale_(scale_);
	get_window()->invalidate_rect( wa, false);
}

void
ImageArea::draw_margins( Cairo::RefPtr<Cairo::Context>& context,
	int width, int height)
{
	context->set_line_width(3.);

	// Draw margins:
	for ( int i = 1; i < SCANNER_CHIPS; i++) {
		context->move_to( i * IMAGE_STRIPS_PER_CHIP * scale_, 0);
		context->rel_line_to( 0, height);
	}
	context->rectangle( 0, 0, width, height);
}

void
ImageArea::draw_numbers(Cairo::RefPtr<Cairo::Context>& context)
{
	Cairo::TextExtents extents;

	context->select_font_face( "Sans", Cairo::FONT_SLANT_NORMAL,
		Cairo::FONT_WEIGHT_NORMAL);
	context->set_font_size(20.);
	
	for ( int i = 0; i < SCANNER_CHIPS; i++) {
		context->get_text_extents( chip_strings[i], extents);

		int sx = i * IMAGE_STRIPS_PER_CHIP * scale_;
		sx += extents.width + extents.x_bearing;
		sx -= 0.5 * i;
		int sy = 3 * extents.height + extents.y_bearing;

		context->move_to( sx, sy);
		context->show_text(chip_strings[i]);
	}
}

void
ImageArea::draw_broken_strips(
	Cairo::RefPtr<Cairo::Context>& context,
	int height)
{
	for ( std::vector<guint>::const_iterator it = broken_strips_.begin();
		it != broken_strips_.end(); ++it) {
		context->move_to( *it * scale_, 0);
		context->rel_line_to( 0, height);
	}
}

void
ImageArea::on_draw_margins(const Glib::RefPtr<Gtk::ToggleAction>& action)
{
	if (pixbuf_) {
		margins_ = action->get_active();
		update_pixmap();
		queue_draw();
	}
}

void
ImageArea::on_draw_broken_strips(const Glib::RefPtr<Gtk::ToggleAction>& action)
{
	if (pixbuf_) {
		if (action->get_active()) {
			Scanner::SharedManager manager = Scanner::Manager::instance();
			broken_strips_ = manager->current_broken_strips();
		}
		else
			broken_strips_.clear();

		update_pixmap();
		queue_draw();
	}
}

void
ImageArea::clear_area()
{
	pos_x_ = 0;
	pos_y_ = 0;
	image_width_ = 0;
	image_height_ = 0;
	width_ = 0;
	height_ = 0;
	vis_width_ = 0;
	vis_height_ = 0;
	x_offset_ = 0;
	y_offset_ = 0;
	scale_ = 1.0;
	zoom_ = ZOOM_HEIGHT;
	margins_ = false;

	pixbuf_.reset();
	pixmap_.reset();
	buf_.clear();
	buf_color_.clear();

	modify_bg( Gtk::STATE_NORMAL, gray);

	queue_draw();
}

Glib::RefPtr<Gdk::Pixbuf>
ImageArea::get_pixbuf() const
{
	return Gdk::Pixbuf::create_from_data( &buf_[0],
		Gdk::COLORSPACE_RGB, false, 8,
		image_width_, image_height_, 3 * image_width_);
}

void
ImageArea::set_popup_menu(Gtk::Menu* menu)
{
	menu_ = menu;
}

bool
ImageArea::pointer_within_image(const GdkEventMotion* event) const
{
	int x = x_offset_ + width_;
	int y = y_offset_ + height_;
	bool within_x = event->x >= x_offset_ && event->x < x;
	bool within_y = event->y >= y_offset_ && event->y < y;

	return (within_x && within_y);
}

} // namepace UI

} //namespace ScanAmati
