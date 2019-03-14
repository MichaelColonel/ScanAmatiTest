AC_INIT( [scanamati], [0.0.1alpha], [pmk@ihep.ru])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([-Wall -Werror foreign gnu])
AC_CONFIG_SRCDIR([src/main.cpp])

AC_PROG_CC
AC_PROG_CXX
AC_ISC_POSIX
AC_PROG_RANLIB
AM_PROG_CC_STDC
AM_PROG_CC_C_O
AM_PROG_AR

dnl****************************************************************************
dnl C structs, typedefs, functions, headers and compiler characteristics checks
dnl****************************************************************************
dnl (running autoscan can help to check if everything's
dnl here; in that case, an empty autoscan*.log should be produced).

AC_HEADER_DIRENT
AC_HEADER_STDBOOL
AC_HEADER_STDC

dnl reasonable guesses for where stuff is installed
if test "x$prefix" = "xNONE"; then
	prefix="/usr/local"
else
	prefix=$prefix
fi

dnl reasonable guesses for where stuff is installed
if test "x$sysconfdir" = "xNONE"; then
	prefix="/dev"
else
	sysconfdir=$sysconfdir
fi

dnl AC_ARG_ENABLE( [debug],
dnl	AS_HELP_STRING([--enable-debug], [Enable debugging code]),
dnl	case[ "${enableval}" in
dnl		yes) debug=true ;;
dnl		no) debug=false ;;
dnl		*) AC_MSG_ERROR(bad value ${enableval} for --enable-debug) ;;
dnl	esac], [debug=false])
dnl AM_CONDITIONAL( DEBUG, test x$debug = xtrue)

AC_ARG_ENABLE( [debug],
	AS_HELP_STRING([--enable-debug], [Enable debugging code]))

if test "x$enable_debug" = "xyes"; then
	CFLAGS="$CFLAGS -DDEBUG -Wall"
	CXXFLAGS="$CXXFLAGS -DDEBUG -Wall"
fi

AM_PATH_GLIB_2_0( [2.12.0],
                  AC_DEFINE_UNQUOTED( HAVE_GLIB_2_0, 1, [glib2 support]),
                  AC_MSG_ERROR([glib2-2.12.0 or better is required]))

PKG_CHECK_MODULES( GLIBMM, glibmm-2.4 >= 2.16.0, ,
                   AC_MSG_ERROR([glibmm-2.16.0 or better is required]))
AC_SUBST(GLIBMM_CFLAGS)
AC_SUBST(GLIBMM_LIBS)

PKG_CHECK_MODULES( GTHREAD, gthread-2.0 >= 2.12.0, ,
                   AC_MSG_ERROR([gthread-2.0 2.12.0 or better is required]))
AC_SUBST(GTHREAD_CFLAGS)
AC_SUBST(GTHREAD_LIBS)

PKG_CHECK_MODULES( SIGCPP, sigc++-2.0 >= 2.2.0, ,
                   AC_MSG_ERROR([sigc++-2.2.0 or better is required]))
AC_SUBST(SIGCPP_CFLAGS)
AC_SUBST(SIGCPP_LIBS)

PKG_CHECK_MODULES( GTKMM, gtkmm-2.4 >= 2.12.0, ,
                   AC_MSG_ERROR([gtkmm-2.12.0 or better is required]))
AC_SUBST(GTKMM_CFLAGS)
AC_SUBST(GTKMM_LIBS)

PKG_CHECK_MODULES( LIBPQ, libpq >= 10.2, ,
                   AC_MSG_ERROR([libpq-10.2 or better is required]))
AC_SUBST(LIBPQ_CFLAGS)
AC_SUBST(LIBPQ_LIBS)

PKG_CHECK_MODULES( LIBPQXX, libpqxx >= 3.1, ,
                   AC_MSG_ERROR([libpqxx-3.1 or better is required]))
AC_SUBST(LIBPQXX_CFLAGS)
AC_SUBST(LIBPQXX_LIBS)

AM_PATH_XMEDCON( [0.9.9.6],
                 AC_DEFINE_UNQUOTED( HAVE_XMEDCON, 1, [(X)MedCon support]),
                 AC_MSG_ERROR([xmedcon-0.9.9.6 or better is required]))

dnl AM_PATH_XINE( [1.1.15],
dnl               AC_DEFINE_UNQUOTED( HAVE_XINE, 1, [xine-lib support]),
dnl               AC_MSG_ERROR([xine-lib-1.1.15 or better is required]))

PKG_CHECK_MODULES([MAGICK], [Magick++ >= 6.5.9])
AC_SUBST([MAGICK_CFLAGS])
AC_SUBST([MAGICK_LIBS])

AC_CHECK_HEADER( [ccmath.h],
                 AC_DEFINE_UNQUOTED( HAVE_CCMATH_H, 1,
                 [Define to 1 if you have the <ccmath.h> header file]))

dnl AC_CHECK_PROG( [have_dcmodify_app], [dcmodify], yes, no)

dnl if test "$have_dcmodify_app" = no; then
dnl 	AC_MSG_ERROR([dcmodify application from DCMTK is required])
dnl fi

AC_CHECK_PROG( [have_storescu_app], [storescu], yes, no)

if test "$have_storescu_app" = no; then
	AC_MSG_ERROR([storescu application from DCMTK is required])
fi

AC_CHECK_PROG( [have_movescu_app], [movescu], yes, no)

if test "$have_movescu_app" = no; then
	AC_MSG_ERROR([movescu application from DCMTK is required])
fi

AC_CHECK_PROG( [have_echoscu_app], [echoscu], yes, no)

if test "$have_echoscu_app" = no; then
	AC_MSG_ERROR([echoscu application from DCMTK is required])
fi

dnl Define directory locations for use in the program
SCANAMATI_DATADIR=[${prefix}/share/scanamati]

dnl pass the variables to automake
AC_SUBST([SCANAMATI_DATADIR])

AC_DEFINE_UNQUOTED( [SCANAMATI_PKGDATADIR], ["$SCANAMATI_DATADIR"],
                    [The scanamati package data directory])

ALL_LINGUAS="ru"
GETTEXT_PACKAGE=$PACKAGE
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED( [GETTEXT_PACKAGE], ["$GETTEXT_PACKAGE"],
                    [The domain to use with gettext])

AM_GLIB_GNU_GETTEXT()
AM_GLIB_DEFINE_LOCALEDIR(SCANAMATI_LOCALEDIR)

sinclude(m4/mysql_loc.m4)
sinclude(m4/ccmath.m4)
sinclude(m4/dcmtk_data_shared_library.m4)

dnl MYSQL_API_LOCATION
CHECK_DCMTK_DATA_SHARED_LIBRARY_LINKAGE

if test "$have_dcmtk_data_shared_library" = yes; then
	DCMDATA_LIBS="-L/usr/lib64 -L/usr/local/lib64 -L/usr/lib64/dcmtk -L/usr/local/lib64/dcmtk \
	-L/usr/lib -L/usr/local/lib -L/usr/lib/dcmtk -L/usr/local/lib/dcmtk \
	-ldcmnet -ldcmdata -loflog -lofstd -ldcmtls -lnsl -lwrap -lpthread -lz"
	DCMDATA_CFLAGS="-DHAVE_CONFIG_H"
	AC_SUBST([DCMDATA_CFLAGS])
	AC_SUBST([DCMDATA_LIBS])
else
	AC_MSG_ERROR([
		DCMTK data shared library is required.
		Check include directory for existence of such files:
		<dcmtk/config/osconfig.h>,
		<dcmtk/dcmdata/dctk.h>,
		<dcmtk/ofstd/ofstring.h>,
		and library directory for existance of libraries:
		libdcmdata, libofstd, liboflog, libz, libpthread.])
fi

CHECK_CCMATH_LIBRARY_LINKAGE

if test "$have_ccmath_library" = yes; then
	CCMATH_LIBS="-lccm"
	AC_SUBST([CCMATH_LIBS])
else
	AC_MSG_ERROR([
		CCMATH library is required.
		Check include directory for existence of the file:
		<ccmath.h>
		and library directory for existance of the library:
		libccm.])
fi

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile
src/Makefile
src/dialogs/Makefile
src/dicom/Makefile
src/image/Makefile
src/palette/Makefile
src/print/Makefile
src/scanner/Makefile
src/widgets/Makefile
ui/Makefile
icons/Makefile
device/Makefile
sounds/Makefile
data/Makefile
docs/Makefile
desktop/Makefile
po/Makefile.in])

AC_OUTPUT

AC_MSG_NOTICE([Configuration complete.])
