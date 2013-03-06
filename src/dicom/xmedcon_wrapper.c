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

#include <config.h>

#ifdef HAVE_XMEDCON
#include <medcon.h>
#include <string.h>
#endif

#include "xmedcon_wrapper.h"

void
mdc_init(void)
{
#ifdef HAVE_XMEDCON
	MdcInit();

	MDC_INFO = MDC_NO;
	MDC_DEBUG = MDC_YES;
	MDC_VERBOSE = MDC_NO;
	MDC_FILE_OVERWRITE = MDC_YES;

	// Warning: Do not use the prefix variable.
	// This global variable is from the (X)MedCon project.
	memset( prefix, 0, sizeof(prefix));
#endif
}

void
mdc_finish(void)
{
#ifdef HAVE_XMEDCON
	MdcFinish();
#endif
}
