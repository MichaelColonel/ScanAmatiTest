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

#include <gtkmm/table.h>
#include <gtkmm/builder.h>

namespace Gtk {
class HScrollbar;
class VScrollbar;
} // namespace Gtk

namespace ScanAmati {

namespace UI {

class ImageArea;
class PaletteArea;

class ScrolledImageArea : public Gtk::Table {

public:
	ScrolledImageArea( BaseObjectType* cobject,
		const Glib::RefPtr<Gtk::Builder>& builder);
	virtual ~ScrolledImageArea();
	ImageArea* image_area_widget() { return image_area_; }
	PaletteArea* palette_area_widget() { return palette_area_; }

	void clear_area();

protected:
	void init_ui();
	void connect_signals();

	void on_adjustment_value_changed();
	void on_image_area_allocation_changed(Gtk::Allocation&);
	void on_scale_changed();

	Glib::RefPtr<Gtk::Builder> builder_; // builder
	Gtk::HScrollbar* scrollbar_width_; // horizontal scrollbar
	Gtk::VScrollbar* scrollbar_height_; // vertical scrollbar
	ImageArea* image_area_; // image drawing area
	PaletteArea* palette_area_; // palette drawing area

	sigc::signal< void, int, int> signal_scroll_position_changed_;
};

} // namespace UI

} // namespace ScanAmati
