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

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "defines.hpp"
#include "manager.hpp"

#include <glibmm/ustring.h>
#include <glibmm/i18n.h>

#include "commands.hpp"

namespace ScanAmati {

namespace Scanner {

void
Manager::open(const Glib::ustring& devname) throw(Error)
{
	fd_ = ::open( devname.c_str(), O_RDWR);
	if (fd_ == -1) {
		throw Error( strerror(errno), errno);
	}
}

void
Manager::close() throw(Error)
{
	if (::close(fd_) == -1) {
		fd_ = -1;
		throw Error( strerror(errno), errno);
	}
	fd_ = -1;
}

size_t
Manager::read( char* buf, size_t count) throw( Exception, Error)
{
	try {
		check();
	}
	catch (const Error&) {
		throw;
	}
	catch (const Exception&) {
		throw;
	}

	ssize_t nread;

	if ((nread = ::read( fd_, buf, count)) == -1)
		throw Error( strerror(errno), errno);

	return static_cast<size_t>(nread);
}

void
Manager::check() throw( Exception, Error)
{
	fd_set rfds;
	struct timeval tv;

	FD_ZERO(&rfds);
	FD_SET( fd_, &rfds);

	tv.tv_sec = SCANNER_DATA_TIMEOUT;
	tv.tv_usec = 0;

	int res = select( fd_ + 1, &rfds, 0, 0, &tv);
	if (res == 0) {
		throw Exception(_("No data within timeout interval."));
	}
	else if (res == -1)
		throw Error( strerror(errno), errno);
}

size_t
Manager::readn( char* buf, size_t count) throw( Exception, Error)
{
	size_t nleft = count, bytes_read = 0;
	char* ptr = buf;
	
	ssize_t nread;

	while (nleft > 0) {
		try {
			check();
		}
		catch (const Error&) {
			throw;
		}
		catch (const Exception&) {
			throw;
		}

		if ((nread = ::read( fd_, ptr, nleft)) == -1)
			throw Error( strerror(errno), errno);
		else if (nread == 0)
			throw Exception(_("No more data."));

		bytes_read += nread;
		nleft -= nread;
		ptr += nread;
	}

	return bytes_read;
}

size_t
Manager::write( const char* buf, size_t count) throw(Error)
{
	ssize_t nwritten;

	if ((nwritten = ::write( fd_, buf, count)) == -1) {
		throw Error( strerror(errno), errno);
	}

	return static_cast<gsize>(nwritten);
}

void
Manager::write_command(Command* com) throw(Error)
{
	std::fill( buffers_.com.data, buffers_.com.data + SCANNER_BUFFER_FULL, 0);
	if (com) {
		gsize size;
		com->fill_buffer( buffers_.com.data, size);

		try {
			write( buffers_.com.buf, size);
			delete com;
			com = 0;
		}
		catch (const Error&) {
			delete com;
			com = 0;
			throw;
		}
	}
}

void
Manager::write_command(const CommandSharedPtr& com) throw(Error)
{
	std::fill( buffers_.com.data, buffers_.com.data + SCANNER_BUFFER_FULL, 0);
	if (com) {
		gsize size;
		com->fill_buffer( buffers_.com.data, size);

		try {
			write( buffers_.com.buf, size);
		}
		catch (const Error&) {
			throw;
		}
	}
}

} // namespace Scanner

} // namespace ScanAmati
