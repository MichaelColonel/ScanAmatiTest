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

#include <gtkmm/drawingarea.h>
#include <gtkmm/radioaction.h>
#include <gtkmm/builder.h>

// files from src directory begin
#include "palette/palette.hpp"
// files from src directory end

namespace Gtk {
class Menu;
}

namespace ScanAmati {

namespace UI {

class PaletteArea : public Gtk::DrawingArea {

public:
	PaletteArea( BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>&);
	virtual ~PaletteArea();
	void on_palette_type_changed( const Glib::RefPtr<Gtk::RadioAction>&,
		Image::PaletteType);
	void set_popup_menu(Gtk::Menu*);
	sigc::signal< void, const Image::Palette*> signal_palette_changed();

protected:
	// Override default signal handler:
	virtual bool on_expose_event(GdkEventExpose*);
	virtual bool on_button_press_event(GdkEventButton*);

private:
	enum { WIDTH = 15, HEIGHT = 100 };
	Image::Palette* palette_;
	Gtk::Menu* menu_;
	sigc::signal< void, const Image::Palette*> signal_palette_changed_;
};

} // namespace UI

} // namespace ScanAmati
