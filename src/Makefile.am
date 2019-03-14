## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the Licence, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

SUBDIRS = dialogs dicom palette scanner widgets print image

bin_PROGRAMS = scanamati

scanamati_SOURCES = \
	application.hpp \
	application.cpp \
	ccmath_wrapper.h \
	ccmath_wrapper.c \
	exceptions.hpp \
	dcmtk_defines.hpp \
	global_strings.hpp \
	utils.hpp \
	utils.cpp \
	main.cpp \
	preferences.hpp \
	preferences.cpp \
	file.hpp \
	file.cpp \
	file_loader.hpp \
	file_loader.cpp \
	file_saver.hpp \
	file_saver.cpp \
	icon_loader.hpp \
	icon_loader.cpp \
	icon_names.hpp \
	main_window.hpp \
	main_window_actions.cpp \
	main_window_action_handlers.cpp \
	main_window.cpp

AM_CPPFLAGS = $(GTKMM_CFLAGS) \
	$(GTHREAD_CFLAGS) \
	$(XINE_CFLAGS) \
	$(DBUS_CFLAGS) \
	$(HAL_CFLAGS) \
	$(MAGICK_CFLAGS) \
	$(LIBPQ_CFLAGS) \
	$(LIBPQXX_CFLAGS)

scanamati_LDADD = \
	$(LIBINTL) \
	$(GTKMM_LIBS) \
	$(GTHREAD_LIBS) \
	$(XMEDCON_LIBS) \
	$(XINE_LIBS) \
	$(MAGICK_LIBS) \
	$(DBUS_LIBS) \
	$(HAL_LIBS) \
	$(DCMDATA_LIBS) \
	$(CCMATH_LIBS) \
	$(LIBPQ_LIBS) \
	$(LIBPQXX_LIBS) \
	dialogs/libdialogs.a \
	widgets/libwidgets.a \
	scanner/libscanner.a \
	dicom/libdicom.a \
	palette/libpalette.a \
	print/libprint.a \
	image/libimage.a
