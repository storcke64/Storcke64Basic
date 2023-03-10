/***************************************************************************

	gbx_stream_pipe.c

	(c) 2000-2017 Benoît Minisini <benoit.minisini@gambas-basic.org>

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2, or (at your option)
	any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
	MA 02110-1301, USA.

***************************************************************************/

#define __STREAM_IMPL_C

#include "gb_common.h"
#include "gb_error.h"
#include "gbx_value.h"
#include "gb_limit.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include "gbx_stream.h"


#define FD (stream->direct.fd)


static int stream_open(STREAM *stream, const char *path, int mode)
{
	int fd;
	int fmode;

	RESTART_SYSCALL(mkfifo(path, 0666))
	{
		if (errno != EEXIST)
			return TRUE;
	}

	fmode = 0;

	switch (mode & GB_ST_MODE)
	{
		case GB_ST_READ: fmode |= O_RDONLY; break;
		case GB_ST_WRITE: fmode |= O_WRONLY; break;
		case GB_ST_READ_WRITE: fmode |= O_RDWR; break;
		default: fmode |= O_RDWR;
	}

	RESTART_SYSCALL(fd = open(path, fmode | O_NONBLOCK)) 
		return TRUE;

	if ((mode & GB_ST_MODE) == GB_ST_READ)
		fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) & ~O_NONBLOCK);
		
	stream->pipe.can_write = ((mode & GB_ST_MODE) & GB_ST_WRITE) != 0;
	stream->common.check_read = TRUE;

	FD = fd;
	return FALSE;
}


static int stream_close(STREAM *stream)
{
	if (close(FD) < 0)
		return TRUE;

	FD = -1;
	return FALSE;
}


static int stream_read(STREAM *stream, char *buffer, int len)
{
	return read(FD, buffer, len);
}


static int stream_write(STREAM *stream, char *buffer, int len)
{
	if (!stream->pipe.can_write)
	{
		errno = EBADF;
		return -1;
	}
	
	return write(FD, buffer, len);
}


static int stream_seek(STREAM *stream, int64_t pos, int whence)
{
	return TRUE;
}


static int stream_tell(STREAM *stream, int64_t *pos)
{
	return TRUE;
}


static int stream_flush(STREAM *stream)
{
	return FALSE;
}


#define stream_eof NULL

#define stream_lof NULL


static int stream_handle(STREAM *stream)
{
	return FD;
}


DECLARE_STREAM(STREAM_pipe);

