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

#include <gtkmm/dialog.h>
#include <gtkmm/builder.h>

struct T_DIMSE_StoreProgress; // progress state
struct T_DIMSE_C_StoreRQ; // original store request
struct T_DIMSE_C_StoreRSP; // final store response

namespace Gtk {
class ProgressBar;
} // namespace Gtk

namespace ScanAmati {

namespace UI {

class MoveDicomDialog : public Gtk::Dialog {
public:
	static MoveDicomDialog* create();
	MoveDicomDialog( BaseObjectType* cobject,
		const Glib::RefPtr<Gtk::Builder>& builder);
	virtual ~MoveDicomDialog();
	virtual void move_progress( void*, T_DIMSE_StoreProgress*,
		T_DIMSE_C_StoreRQ*,	char*, DcmDataset**,
		T_DIMSE_C_StoreRSP*, DcmDataset**);
	void move_query( const DcmDataset&, const std::string& dir);
	sigc::signal< void, bool, const Glib::ustring&> signal_move_result();

protected:
	// UI methods:
	void init_ui();
	void connect_signals();
	virtual void query(const DcmDataset&);

	// Signal handlers:
	virtual void on_response(int);

	// Members:
	Glib::RefPtr<Gtk::Builder> builder_;
	Gtk::ProgressBar* progressbar_;
	std::string directory_;
	Glib::ustring message_;
	sigc::signal< void, bool, const Glib::ustring&> signal_move_result_;
};

} // namespace UI

} // namespace ScanAmati
