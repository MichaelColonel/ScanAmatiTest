dnl @synopsis CHECK_DCMTK_DATA_SHARED_LIBRARY_LINKAGE
dnl
dnl This macro tries to check DCMTK data shared library linkage.
dnl

AC_DEFUN( [CHECK_DCMTK_DATA_SHARED_LIBRARY_LINKAGE],
[
	have_dcmtk_data_shared_library=no

	AC_CACHE_CHECK(if DCMTK data library installed,
		ac_cv_dcmtk_check,
		[AC_LANG_SAVE
		AC_LANG_CPLUSPLUS
		ac_save_CPPFLAGS="$CPPFLAGS"
		CPPFLAGS="$CPPFLAGS -DHAVE_CONFIG_H -I/usr/include -I/usr/local/include \
			-L/usr/lib -L/usr/lib64 -L/usr/local/lib -L/usr/local/lib64 \
			-L/usr/lib/dcmtk -L/usr/lib64/dcmtk -L/usr/local/lib/dcmtk -L/usr/local/lib64/dcmtk \
			-ldcmdata -loflog -lofstd -lpthread -lz"

		AC_TRY_COMPILE([
			#include <dcmtk/dcmdata/dctk.h>
			#include <dcmtk/ofstd/ofstring.h>
			DcmFileFormat fileformat; OFString ofstring;],,
			ac_cv_dcmtk_check=yes,
			ac_cv_dcmtk_check=no)
			AC_LANG_RESTORE
			]
		)

	CPPFLAGS="$ac_save_CPPFLAGS"
	if test "$ac_cv_dcmtk_check" = yes; then
		have_dcmtk_data_shared_library=yes
		AC_DEFINE(HAVE_DCMTK_DATA_SHARED_LIBRARY, 1,
			[Define to 1 if DCMTK data headers and libraries are installed. ])
	fi
]) dnl CHECK_DCMTK_DATA_LIBRARY_SHARED_LINKAGE
