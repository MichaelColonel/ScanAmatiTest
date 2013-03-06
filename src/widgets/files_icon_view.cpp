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
#include <fstream>
#include <gtkmm/menu.h>

#include <Magick++.h>

// files from src directory begin
#include "palette/palette.hpp"
// files from src directory end

#include "files_icon_view.hpp"

namespace {

void
destroy_pixbuf(const guint8* data)
{
	delete [] data;
	data = 0;
}

Gdk::Pixbuf::SlotDestroyData destroy_slot = sigc::ptr_fun(&destroy_pixbuf);
	
} // namespace

namespace ScanAmati {

namespace UI {

FilesIconView::FilesIconView( BaseObjectType* cobject,
	const Glib::RefPtr<Gtk::Builder>& builder)
	:
	Gtk::IconView(cobject),
	builder_(builder),
	menu_view_(0),
	menu_icons_(0),
	item_selected_(false)
{
	init_ui();
	connect_signals();
}

FilesIconView::~FilesIconView()
{
	liststore_icons_->clear();
}

void
FilesIconView::connect_signals()
{
}

bool
FilesIconView::on_button_press_event(GdkEventButton* event)
{
	if ((event->type == GDK_BUTTON_PRESS) && (event->button == 3)) {
		Gtk::Menu* menu = item_selected_ ? menu_icons_ : menu_view_;

		menu->popup( event->button, event->time);
	}
	return IconView::on_button_press_event(event);
}

bool
FilesIconView::on_motion_notify_event(GdkEventMotion* event)
{
	pointer_path_ = get_path_at_pos( event->x, event->y);

	item_selected_ = (pointer_path_.empty() || current_path_.empty()) ? false :
		(pointer_path_ == current_path_);

	return IconView::on_motion_notify_event(event);
}

void
FilesIconView::on_item_activated(const Gtk::TreeModel::Path& path)
{
	current_path_ = path;
	IconView::on_item_activated(path);
}

void
FilesIconView::on_selection_changed()
{
	std::vector<Gtk::TreeModel::Path> paths = get_selected_items();
	for ( std::vector<Gtk::TreeModel::Path>::const_iterator iter = paths.begin();
		iter != paths.end(); ++iter) {
		if (current_path_.empty() || (current_path_ != *iter)) {
			item_selected_ = true;
			current_path_ = *iter;
			Gtk::TreeModel::Row row = *liststore_icons_->get_iter(*iter);

			const Image::SummaryData& image = row[model_columns.image_data];
			const DICOM::SummaryInfo& info = row[model_columns.dicom_info];
			Image::DataType data_type = current_data_available(image);

			signal_image_data_clicked_( Image::DATA_FOR_PRESENTATION,
				image);
			signal_dicom_info_clicked_(info);
			signal_state_type_clicked_(row[model_columns.state]);
			signal_data_type_clicked_(data_type);
		}
		else
			item_selected_ = false;
	}

	IconView::on_selection_changed();
}

void
FilesIconView::init_ui()
{
	liststore_icons_ = Gtk::ListStore::create(model_columns);

	set_model(liststore_icons_);

	set_pixbuf_column(model_columns.icon);
	set_text_column(model_columns.label);
	set_tooltip_column(2);
}

void
FilesIconView::add_file( const std::string& filename,
	File& file, bool from_pacs)
{
	Image::SummaryData& data = file.image_data();
	DICOM::SummaryInfo& info = file.dicom_info();
	Glib::RefPtr<Gdk::Pixbuf> icon = create_icon_pixbuf(
		data.presentation_data(), data.image_buffer(), 64);

	Gtk::TreeRow row = *(liststore_icons_->append());
	row[model_columns.icon] = icon;
	row[model_columns.label] = info.get_label_text();
	row[model_columns.tooltip_label] = info.get_tooltip_text();
	row[model_columns.filename] = filename;
	row[model_columns.dicom_info] = info;
	row[model_columns.image_data] = data;
	row[model_columns.state] = (from_pacs) ? STATE_PACS_DATA : STATE_FILE_DATA;
}

void
FilesIconView::add_image_data(const Image::SummaryData& image_data)
{
	Glib::RefPtr<Gdk::Pixbuf> icon = create_icon_pixbuf(
		image_data.presentation_data(), image_data.image_buffer(), 64);

	Gtk::TreeRow row = *(liststore_icons_->append());
	row[model_columns.icon] = icon;
	row[model_columns.tooltip_label] = "";
	row[model_columns.filename] = "";
	row[model_columns.dicom_info] = new_dicom_info_;
	row[model_columns.image_data] = image_data;
	row[model_columns.state] = STATE_NEW_DATA;
}

bool
FilesIconView::get_current_data( Image::SummaryData& data,
	DICOM::SummaryInfo& info)
{
	bool res = false;
	if (!current_path_.empty()) {
		Gtk::TreeRow row = *(liststore_icons_->get_iter(current_path_));
		data = row[model_columns.image_data];
		info = row[model_columns.dicom_info];
		res = true;
	}
	return res;
}

bool
FilesIconView::get_current_data(DICOM::SummaryInfo& info)
{
	bool res = false;
	if (!current_path_.empty()) {
		Gtk::TreeRow row = *(liststore_icons_->get_iter(current_path_));
		info = row[model_columns.dicom_info];
		res = true;
	}
	return res;
}

bool
FilesIconView::get_current_data( DICOM::Dataset& dataset,
	Image::DataType type)
{
	bool res = false;

	if (!current_path_.empty()) {
		Gtk::TreeRow row = *(liststore_icons_->get_iter(current_path_));
		const Image::SummaryData& data = row[model_columns.image_data];

		if (data.check_available_data(type)) {
			DcmDataset* set = &dataset;
			DICOM::SummaryInfo info = row[model_columns.dicom_info];
			Image::DataSharedPtr image = data.get_image();
			info.save(set);
			if (File::write_image_data( set, image))
				res = true;
		}
	}
	return res;
}

bool
FilesIconView::get_current_filename(std::string& filename)
{
	bool res = false;
	if (!current_path_.empty()) {
		Gtk::TreeRow row = *(liststore_icons_->get_iter(current_path_));
		filename = row[model_columns.filename];
		res = true;
	}
	return res;
}

void
FilesIconView::remove_current()
{
	if (!current_path_.empty()) {
		Gtk::TreeIter iter = liststore_icons_->get_iter(current_path_);
		liststore_icons_->erase(iter);
		current_path_.clear();
		pointer_path_.clear();
		signal_images_cleaned_();
		item_selected_ = false;
	}
}

void
FilesIconView::change_current_filename(const std::string& filename)
{
	Gtk::TreeRow row = *(liststore_icons_->get_iter(current_path_));
	row[model_columns.filename] = filename;
	row[model_columns.state] = STATE_FILE_SAVED;
	signal_state_type_clicked_(STATE_FILE_SAVED);
}

void
FilesIconView::change_current_dicom_info(const DICOM::SummaryInfo& info)
{
	Gtk::TreeRow row = *(liststore_icons_->get_iter(current_path_));
	row[model_columns.dicom_info] = info;
	signal_dicom_info_clicked_(info);
}

void
FilesIconView::clear_all()
{
	liststore_icons_->clear();
	current_path_.clear();
	pointer_path_.clear();
	signal_images_cleaned_();
	item_selected_ = false;
}

void
FilesIconView::set_new_dicom_info(const DICOM::SummaryInfo& info)
{
	new_dicom_info_ = info;
}

void
FilesIconView::set_palette(const Image::Palette*)
{
}

Glib::RefPtr<Gdk::Pixbuf>
FilesIconView::create_icon_pixbuf( const Image::DataSharedPtr& img,
	const std::vector<guint8>& buf, unsigned int icon_size)
{
	Glib::RefPtr<Gdk::Pixbuf> pixbuf;

	if (img) {
		Magick::Image icon( img->width(), img->height(), "I",
			Magick::CharPixel, &buf[0]);
		Glib::ustring str = Glib::ustring::compose( "%1x%1\>",
			Glib::ustring::format(icon_size));

		Magick::Geometry geom(str.c_str());
		icon.filterType(Magick::CubicFilter);
#if (MagickLibVersion >= 0x660 && MagickLibVersion <= 0x669) 
		icon.resize(geom);
#else
		icon.scale(geom);
#endif

		unsigned int w = icon.columns();
		unsigned int h = icon.rows();
		const Magick::PixelPacket* pixel = icon.getConstPixels( 0, 0, w, h);
		guint8* d = new guint8[3 * w * h];
		for ( unsigned int i = 0; i < w * h; ++i) {
			guint8 pix = UCHAR_MAX * (double(pixel++->red) / USHRT_MAX);
			d[3 * i + 0] = d[3 * i + 1] = d[3 * i + 2] = pix;
		}
		pixbuf = Gdk::Pixbuf::create_from_data( d, Gdk::COLORSPACE_RGB,
			false, 8, w, h, 3 * w, destroy_slot);
	}
	return pixbuf;
}

Image::DataType
FilesIconView::current_data_available(const Image::SummaryData& summary)
{
	Image::DataType data = Image::DATA_FOR_PRESENTATION;

	if (summary.check_available_data(Image::DATA_RAW))
		data = Image::DATA_RAW;
	else if (summary.check_available_data(Image::DATA_FOR_PROCESSING))
		data = Image::DATA_FOR_PROCESSING;
	else if (summary.check_available_data(Image::DATA_FOR_PRESENTATION))
		data = Image::DATA_FOR_PRESENTATION;

	return data;
}

sigc::signal< void, Image::DataType, const Image::SummaryData&>
FilesIconView::signal_image_data_clicked()
{
	return signal_image_data_clicked_;
}

sigc::signal< void, const DICOM::SummaryInfo&>
FilesIconView::signal_dicom_info_clicked()
{
	return signal_dicom_info_clicked_;
}

sigc::signal< void, FilesIconView::StateType>
FilesIconView::signal_state_type_clicked()
{
	return signal_state_type_clicked_;
}

sigc::signal< void, Image::DataType>
FilesIconView::signal_data_type_clicked()
{
	return signal_data_type_clicked_;
}

sigc::signal<void>
FilesIconView::signal_images_cleaned()
{
	return signal_images_cleaned_;
}

} // namepace UI

} //namespace ScanAmati
