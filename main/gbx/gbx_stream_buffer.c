/***************************************************************************

	gbx_stream_buffer.c

	(c) 2000-2017 Benoît Minisini <g4mba5@gmail.com>

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

#include "gbx_exec.h"
#include "gbx_stream.h"


#define FD (stream->buffer.file)


static int stream_open(STREAM *stream, const char *path, int mode)
{
	FILE *file;
	char *fmode;
	struct stat info;
	int fd;

	if (mode & GB_ST_CREATE)
		fmode = "w+";
	else if (mode & GB_ST_APPEND)
		fmode = "a+";
	else if (mode & GB_ST_WRITE)
		fmode = "r+";
	else
		fmode = "r";

	file = fopen(path, fmode);
	if (file == NULL)
		return TRUE;

	fd = fileno(file);

	if (fstat(fd, &info) < 0)
	{
		fclose(file);
		return TRUE;
	}

	if (S_ISDIR(info.st_mode))
	{
		fclose(file);
		errno = EISDIR;
		return TRUE;
	}

	//fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | FD_CLOEXEC);

	stream->common.available_now = TRUE;
	FD = file;
	return FALSE;
}


static int stream_close(STREAM *stream)
{
	FILE *f = FD;
	
	if (!f)
		return TRUE;

	FD = NULL;
	
	return fclose(f) < 0;
}


static int stream_read(STREAM *stream, char *buffer, int len)
{
	int eff;
	
	if (!FD)
		return TRUE;

	eff = (int)fread(buffer, 1, len, FD);
	if (eff < len)
	{
		if (ferror(FD) == 0)
			errno = 0;
	}
	
	return eff;

	/*
	while (len > 0)
	{
		len_read = Min(len, MAX_IO);
		eff_read = fread(buffer, 1, len_read, FD);

		if (eff_read > 0)
		{
			STREAM_eff_read += eff_read;
			len -= eff_read;
			buffer += eff_read;
		}

		if (eff_read < len_read)
		{
			if (feof(FD))
			{
				errno = 0;
				return TRUE;
			}
			if (ferror(FD) && errno != EINTR)
				return TRUE;
		}
	}

	return FALSE;
	*/
}


static int stream_flush(STREAM *stream)
{
	if (!FD)
		return TRUE;

	return (fflush(FD) != 0);
}


static int stream_write(STREAM *stream, char *buffer, int len)
{
	if (!FD)
		return TRUE;

	return fwrite(buffer, 1, len, FD);

	/*while (len > 0)
	{
		len_write = Min(len, MAX_IO);
		eff_write = fwrite(buffer, 1, len_write, FD);

		if (eff_write < len_write)
		{
			if (ferror(FD) && errno != EINTR)
				return TRUE;
		}

		len -= eff_write;
		buffer += eff_write;
	}

	if (EXEC_debug)
		return stream_flush(stream);
	else
		return FALSE;*/
}


static int stream_seek(STREAM *stream, int64_t pos, int whence)
{
	if (!FD)
		return TRUE;

	return (fseek(FD, (off_t)pos, whence) != 0);
}


static int stream_tell(STREAM *stream, int64_t *pos)
{
	if (!FD)
		return TRUE;

	*pos = (int64_t)ftell(FD);
	return (*pos < 0);
}


static int stream_eof(STREAM *stream)
{
	int c;

	if (!FD)
		return TRUE;

	c = fgetc(FD);
	if (c == EOF)
		return TRUE;

	ungetc(c, FD);
	return FALSE;
}


static int stream_lof(STREAM *stream, int64_t *len)
{
	struct stat info;

	if (!stream->common.available_now)
		return TRUE;
	
	if (!FD || fstat(fileno(FD), &info) < 0)
		return TRUE;

	*len = info.st_size;
	return FALSE;
}


static int stream_handle(STREAM *stream)
{
	if (FD)
		return fileno(FD);
	else
		return -1;
}



DECLARE_STREAM(STREAM_buffer);
