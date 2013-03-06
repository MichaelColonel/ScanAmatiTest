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

#include <vector>
#include <climits>
#include <gtkmm/menu.h>

#include "palette_area.hpp"

#include "application.hpp"

namespace ScanAmati {

namespace UI {
	
PaletteArea::PaletteArea( BaseObjectType* cobject,
	const Glib::RefPtr<Gtk::Builder>&)
	:
	Gtk::DrawingArea(cobject),
	palette_(0),
	menu_(0)
{
}

PaletteArea::~PaletteArea()
{
	if (palette_) delete palette_;
}

bool
PaletteArea::on_expose_event(GdkEventExpose* event)
{
	// This is where we draw on the window
	Glib::RefPtr<Gdk::Window> window = get_window();

	int height = get_allocation().get_height();

	if (window) {
		std::vector<guint8> data(3 * height * WIDTH);

		for ( int pix = 0, i = height; i > 0; i--) {
			guint8 a = (UCHAR_MAX * i) / height;
			for ( int j = 0; j < WIDTH; j++, pix++) {
				if (palette_) {
					data[pix * 3 + 0] = palette_->rgb(3 * a + 0);
					data[pix * 3 + 1] = palette_->rgb(3 * a + 1);
					data[pix * 3 + 2] = palette_->rgb(3 * a + 2);
				}
				else
					data[pix * 3 + 0] = data[pix * 3 + 1] = data[pix * 3 + 2] = a;
			}
		}

		Glib::RefPtr<Gdk::Pixbuf> pixbuf = Gdk::Pixbuf::create_from_data(
			&data[0], Gdk::COLORSPACE_RGB, false, 8, WIDTH, height,
			3 * WIDTH);

		Glib::RefPtr<Gdk::Pixmap> pixmap = Gdk::Pixmap::create( get_window(),
			WIDTH, height, -1);
		
		pixmap->draw_pixbuf( get_style()->get_black_gc(), pixbuf,
			0, 0, 0, 0, WIDTH, height, Gdk::RGB_DITHER_NONE, 0, 0);

		window->draw_drawable( get_style()->get_black_gc(), pixmap,
			0, 0, 0, 0, WIDTH, height);

		return true;
	}

	return false;
}

bool
PaletteArea::on_button_press_event(GdkEventButton* event)
{
	bool res = false;
	if ((event->type == GDK_BUTTON_PRESS) && (event->button == 3)) {
		menu_->popup( event->button, event->time);
		res = true;
	}
	return res;
}

void
PaletteArea::on_palette_type_changed(
	const Glib::RefPtr<Gtk::RadioAction>& action, Image::PaletteType type)
{
	if (action->get_active()) {
		if (!palette_)
			palette_ = Image::Palette::create(type);
		else
			palette_->fill(type);

		signal_palette_changed_(palette_);

		queue_draw();
	}
}

void
PaletteArea::set_popup_menu(Gtk::Menu* menu)
{
	menu_ = menu;
}
	
sigc::signal< void, const Image::Palette*>
PaletteArea::signal_palette_changed()
{
	return signal_palette_changed_;
}

} // namepace UI

} //namespace ScanAmati
