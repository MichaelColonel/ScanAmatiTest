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

#include <glibmm/i18n.h>
#include <gtkmm/progressbar.h>

// files from src directory begin
#include "scanner/state.hpp"
// files from src directory end

#include "status_bar.hpp"

namespace ScanAmati {

namespace UI {

Statusbar::Statusbar( BaseObjectType* cobject,
	const Glib::RefPtr<Gtk::Builder>& builder)
	:
	Gtk::Statusbar(cobject),
	builder_(builder),
	progressbar_(0)
{
	init_ui();
	connect_signals();
}

Statusbar::~Statusbar()
{
}

void
Statusbar::init_ui()
{
	builder_->get_widget( "progressbar", progressbar_);
}

void
Statusbar::connect_signals()
{
}

void
Statusbar::set_progress(double prog)
{
	progressbar_->set_fraction(prog);
}

void
Statusbar::set_pulse_progress()
{
	progressbar_->pulse();
}

void
Statusbar::set_text(const Glib::ustring& msg)
{
	pop();
	push(msg);
}

void
Statusbar::update_scanner_state(const Scanner::State& state)
{
	Scanner::ManagerState mstate = state.manager_state();

	switch (mstate.run()) {
	case RUN_INITIATION:
		set_text(_("Initiating the scanner."));
		set_progress(mstate.progress());
		break;
	case RUN_IMAGE_ACQUISITION:
	{
		switch (mstate.process()) {
		case PROCESS_EXPOSURE:
		case PROCESS_START:
			set_text(_("Exposure has been started."));
			set_progress(mstate.progress());
			break;
		case PROCESS_ACQUISITION:
			set_text(_("Acquisition has been started."));
			set_progress(mstate.progress());
			break;
		case PROCESS_PARKING:
			set_text(_("Parking has been started."));
			set_progress(mstate.progress());
			break;
		case PROCESS_ABORTED:
			set_text(_("Image acquisition has been aborted."));
			set_progress(mstate.progress());
		default:
			break;
		}
	}
		break;
	case RUN_BACKGROUND:
		set_text(_("Scanner has been connected."));
		set_progress(0);
		break;
	case RUN_LINING_ACQUISITION:
		switch (mstate.process()) {
		case PROCESS_START:
			set_text(_("Lining has been started."));
			set_progress(0);
			break;
		case PROCESS_ACQUISITION:
			set_text(_("Lining acquisition has been started."));
			set_progress(mstate.progress());
			break;
		case PROCESS_FINISH:
			set_text(_("Lining has been finished."));
			set_progress(0);
			break;
		default:
			break;
		}
		break;
	case RUN_COMMANDS:
		set_text(_("Executing commands..."));
		set_progress(mstate.progress());
		break;
	case RUN_IMAGE_RECONSTRUCTION:
		set_text(_("Image reconstruction in progress..."));
		switch (mstate.process()) {
		case PROCESS_START:
			set_pulse_progress();
			break;
		case PROCESS_FINISH:
		default:
			set_progress(0);
			break;
		}
		break;
	case RUN_NONE:
	default:
		set_text(_("Scanner has been disconnected."));
		set_progress(0);
		break;
	}
}

} // namespace UI

} // namespace ScanAmati
