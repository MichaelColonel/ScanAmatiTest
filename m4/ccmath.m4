dnl @synopsis CHECK_CCMATH_LIBRARY_LINKAGE
dnl 
dnl This macro tries to check linkage of the CCMATH library.
dnl

AC_DEFUN( [CHECK_CCMATH_LIBRARY_LINKAGE],
[
	have_ccmath_library=no

	AC_CACHE_CHECK(if CCMATH library installed,
		ac_cv_ccmath_check,
		[AC_LANG_SAVE
		AC_LANG_C
		ac_save_CFLAGS="$CFLAGS"
		CFLAGS="$CFLAGS -lm -lccm"

		AC_TRY_COMPILE([
			#include <math.h>
			#include <ccmath.h>
			double (*func)( double *, double *, int, int, int *) = &qrlsq;],,
			ac_cv_ccmath_check=yes, ac_cv_ccmath_check=no)
			AC_LANG_RESTORE
			CFLAGS="$ac_save_CFLAGS"
		]
		)

	if test "$ac_cv_ccmath_check" = yes; then
		have_ccmath_library=yes
		AC_DEFINE(HAVE_CCMATH_LIBRARY, 1,
			[Define to 1 if ccmath headers and libraries installed. ])
	fi
]) dnl CHECK_CCMATH_LIBRARY_LINKAGE
