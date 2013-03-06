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

#include <vector>
#include <gtkmm/radioaction.h>
#include <gtkmm/builder.h>
#include <gtkmm/drawingarea.h>

// files from src directory begin
#include "image/summary_data.hpp"
// files from src directory end

namespace Gtk {
class Menu;
}

namespace ScanAmati {

namespace Image {

class Palette;

} // namespace Image

namespace UI {

enum ZoomType {
	ZOOM_WIDTH,
	ZOOM_HEIGHT,
	ZOOM_100,
	ZOOM_200,
	ZOOM_300
};

class ImageArea : public Gtk::DrawingArea {

public:
	ImageArea( BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>&);
	virtual ~ImageArea();
	void on_zoom(ZoomType);
	void on_draw_margins(const Glib::RefPtr<Gtk::ToggleAction>&);
	void on_draw_broken_strips(const Glib::RefPtr<Gtk::ToggleAction>&);
	void set_broken_strips(const std::vector<guint>& strips);
	void clear_area();
	void get_image_size( int& width, int& height) const;
	Glib::RefPtr<Gdk::Pixbuf> get_pixbuf() const;
	void set_palette(const Image::Palette*);
	void set_image_data( Image::DataType, const Image::SummaryData& data);

	sigc::signal< void, double> signal_scale_changed();
	void set_scroll_to( int, int);
	void set_popup_menu(Gtk::Menu*);

protected:
	// Override default signal handler:
	virtual void on_realize();
	virtual bool on_expose_event(GdkEventExpose*);
	virtual bool on_button_press_event(GdkEventButton*);
//	virtual bool on_scroll_event(GdkEventScroll*);
	virtual bool on_motion_notify_event(GdkEventMotion*);
	virtual bool on_leave_notify_event(GdkEventCrossing*);
	virtual bool on_enter_notify_event(GdkEventCrossing*);
	virtual bool on_configure_event(GdkEventConfigure*);
//	virtual void on_size_allocate(Gtk::Allocation&);

private:
	void update_image_data();
	void update_pixbuf();
	void update_pixmap();
	void update_image_visible_margins( int width, int height);
	void update_scale( int width, int height);
	void draw_margins( Cairo::RefPtr<Cairo::Context>&, int, int);
	void draw_numbers( Cairo::RefPtr<Cairo::Context>&);
	void draw_broken_strips( Cairo::RefPtr<Cairo::Context>&, int);
	bool pointer_within_image(const GdkEventMotion*) const;

	// Signals
	sigc::signal< void, double> signal_scale_;

	int pos_x_; // x scroll position
	int pos_y_; // y scroll position

	int image_width_; // image actual width (pixels)
	int image_height_; // image actual height (pixels)

	int width_; // scaled image width (pixels)
	int height_; // scaled image height (pixels)

	int vis_width_; // visible part of width
	int vis_height_; // visible part of height

	int x_offset_; // offset of horizontal image start
	int y_offset_; // offset of vertical image start

	double scale_; // image scale (1.0 is actual scale)

	Glib::RefPtr<Gdk::Pixbuf> pixbuf_;
	Glib::RefPtr<Gdk::Pixmap> pixmap_;

	ZoomType zoom_;
	bool margins_;
	std::vector<guint> broken_strips_;

	std::vector<guint8> buf_;
	std::vector<guint8> buf_color_;

	const Image::Palette* palette_;
	Gtk::Menu* menu_;
};

} // namespace UI

} // namespace ScanAmati
