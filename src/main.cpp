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
#include <cstring>
#include <csignal>

#include <gtkmm/main.h>

#include <glibmm/optionentry.h>
#include <glibmm/optiongroup.h>
#include <glibmm/fileutils.h>

#include <glib/gi18n.h>
#include <glib/gstdio.h>

#include "application.hpp"
#include "exceptions.hpp"
#include "icon_loader.hpp"
#include "main_window.hpp"
#include "utils.hpp"

#include <config.h>

namespace ScanAmati {

/** \brief Handler for various kill signals */
void
signal_handler(int signal_number)
{
	switch (signal_number) {
	case SIGTERM:
		OFLOG_DEBUG( app.log, "SIGTERM signal has been recieved.");
		break;
	case SIGINT:
		OFLOG_DEBUG( app.log, "SIGINT signal has been recieved.");
		break;
	case SIGQUIT:
		OFLOG_DEBUG( app.log, "SIGQUIT signal has been recieved.");
		break;
	case SIGHUP:
		OFLOG_DEBUG( app.log, "SIGHUP signal has been recieved.");
		break;
	default:
		OFLOG_DEBUG( app.log, "Unknown signal has been recieved.");
		break;
	}
	Gtk::Main::quit();
}

/**
 * Checks for and creates if necessary the following directories and files:
 *
 * ~/.config/
 * ~/.config/scanamati/
 */
void
setup_rc_dir()
{
	if (!Glib::file_test( Glib::get_user_config_dir(), Glib::FILE_TEST_EXISTS)) {
		if (g_mkdir( Glib::get_user_config_dir().c_str(), S_IRWXU) == -1) {
			g_warning( "Could not create %s!",
				Glib::get_user_config_dir().c_str());
			return;
		}
	}
	if (!file_test( get_rc_dir(), Glib::FILE_TEST_EXISTS)) {
		if (g_mkdir( get_rc_dir().c_str(), S_IRWXU) < 0) {
			g_warning( "Could not create %s!", get_rc_dir().c_str());
			return;
		}
	}
}

/**
 * Command-line option parser.
 */
class OptionGroup : public Glib::OptionGroup {
public:
	OptionGroup();

	bool arg_version;
	bool arg_verbose;
	bool arg_extend;

	Glib::OptionGroup::vecustrings remaining_list;
};

OptionGroup::OptionGroup()
	:
	Glib::OptionGroup(
		_("ScanAmati"),
		_("ScanAmati options"),
		_("Command-line options for scanamati")),
	arg_version(false),
	arg_verbose(false),
	arg_extend(false)
{
	Glib::OptionEntry entry_version, entry_verbose, entry_extend, entry_remaining;

	entry_version.set_long_name("version");
	entry_version.set_short_name('V');
	entry_version.set_description(_("Show version and exit"));

	add_entry( entry_version, arg_version);

	entry_verbose.set_long_name("verbose");
	entry_verbose.set_short_name('v');
	entry_verbose.set_description(_("Set the programm be verbose"));

	add_entry( entry_verbose, arg_verbose);

	entry_extend.set_long_name("extend");
	entry_extend.set_short_name('x');
	entry_extend.set_description(_("Set the programm to the extend mode"));

	add_entry( entry_extend, arg_extend);

	entry_remaining.set_long_name(G_OPTION_REMAINING);
	entry_remaining.set_arg_description(G_OPTION_REMAINING);

	add_entry( entry_remaining, remaining_list);

	set_translation_domain(GETTEXT_PACKAGE);
}

} // namespace Scanamati

int
main( int argc, char** argv)
{
	// Make this application use the current locale for _() translation:
	// (SCANAMATI_LOCALEDIR is defined in the Makefile.am)
	bindtextdomain( GETTEXT_PACKAGE, SCANAMATI_LOCALEDIR);
	bind_textdomain_codeset( GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);

	try {
		Glib::OptionContext option_context(_("[FILE]"));
		ScanAmati::OptionGroup option_group;

		option_context.set_main_group(option_group);
		option_context.set_translation_domain(GETTEXT_PACKAGE);

		Gtk::Main kit( argc, argv, option_context);

		if (option_group.arg_version) {
			std::cout << PACKAGE_NAME << " " << PACKAGE_VERSION << std::endl;
			return EXIT_SUCCESS;
		}

		if (option_group.arg_verbose) {
			ScanAmati::app.debug = true;
			OFLog::configure(OFLogger::DEBUG_LOG_LEVEL); // be verbose with me

			std::cout << "  remaining command line options = ";
			for ( Glib::OptionGroup::vecustrings::const_iterator iter =
				option_group.remaining_list.begin();
				iter != option_group.remaining_list.end(); ++iter) {
					std::cout << *iter << ", ";
			}
			std::cout << std::endl;
		}

		if (option_group.arg_extend)
			ScanAmati::app.extend = true;

		ScanAmati::setup_rc_dir();

		ScanAmati::UI::IconLoader::register_stock_items();

		ScanAmati::application.init();

		ScanAmati::app.main_window =
			ScanAmati::UI::MainWindow::create(option_group.remaining_list);

		/* Signal handlers */
		struct sigaction sa_term;

		memset( &sa_term, 0, sizeof(sa_term));
		sa_term.sa_handler = &ScanAmati::signal_handler;

		sigaction( SIGTERM, &sa_term, NULL);
		sigaction( SIGINT, &sa_term, NULL);
		sigaction( SIGQUIT, &sa_term, NULL);
		sigaction( SIGHUP, &sa_term, NULL);

		if (ScanAmati::app.main_window) {
			Gtk::Main::run(*ScanAmati::app.main_window);

			delete ScanAmati::app.main_window;
			ScanAmati::app.main_window = 0;
		}

		ScanAmati::application.finish();
	}
	catch (const ScanAmati::Exception& ex) {
		std::cerr << _("ScanAmati error:") << " " << ex.what() << std::endl;
		return EXIT_FAILURE;
	}
	catch (const Glib::OptionError& ex) {
		std::cerr << _("Command-line option error:") << " "
			<< ex.what() << std::endl;
		return EXIT_FAILURE;
	}
	catch (const Glib::ThreadError& ex) {
		std::cerr << _("Glib thread error:") << " " << ex.what() << std::endl;
		return EXIT_FAILURE;
	}
	catch (...) {
		std::cerr << _("Unhandled exception [type unknown]") << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
