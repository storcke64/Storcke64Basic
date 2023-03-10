/***************************************************************************

  gbx_stream.c

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

#define __GBX_STREAM_C

#include "gb_common.h"

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <sys/ioctl.h>

#include "gb_common_buffer.h"
#include "gb_common_swap.h"
#include "gb_common_check.h"
#include "gb_error.h"
#include "gbx_value.h"
#include "gb_limit.h"
#include "gbx_exec.h"
#include "gbx_project.h"
#include "gb_file.h"
#include "gambas.h"
#include "gbx_regexp.h"
#include "gbx_api.h"
#include "gbx_string.h"
#include "gbx_watch.h"
#include "gbx_c_array.h"
#include "gbx_c_collection.h"
#include "gbx_struct.h"
#include "gbx_c_file.h"
#include "gbx_stream.h"

#define EXTRA(_stream) ((_stream)->common.extra)

static CFILE *_temp_stream = NULL;
static STREAM *_temp_save = NULL;
static int _temp_level;

#if DEBUG_STREAM
static unsigned char _tag = 0;
static int _nopen = 0;
#endif

static void THROW_SERIAL(void)
{
	THROW(E_SERIAL);
}

void STREAM_exit(void)
{
#if DEBUG_STREAM
	if (_nopen)
		ERROR_warning("%d streams yet opened", _nopen);
#endif
	OBJECT_UNREF(_temp_stream);
}

static STREAM_EXTRA *ENSURE_EXTRA(STREAM *stream)
{
	if (!stream->common.extra)
		ALLOC_ZERO(&stream->common.extra, sizeof(STREAM_EXTRA));
	return stream->common.extra;
}

static void wait_for_fd_ready_to_read(int fd)
{
	if (fd >= 0)
		WATCH_process(fd, -1, -1, 0);
}

bool STREAM_in_archive(const char *path)
{
	ARCHIVE *arch;

	if (FILE_is_relative(path))
	{
		ARCHIVE_get_current(&arch);
		if (arch->name || EXEC_arch) // || !FILE_exist_real(path)) - Why that test ?
			return TRUE;
	}

	return FALSE;
}

bool STREAM_get_readable(STREAM *stream, int *len)
{
	int fd;
	off_t off;
	off_t end;

	fd = STREAM_handle(stream);
	if (fd < 0)
		return TRUE;

//_IOCTL:

	#ifdef FIONREAD

	if (!stream->common.no_fionread)
	{
		if (ioctl(fd, FIONREAD, len) >= 0)
			return FALSE;

		stream->common.no_fionread = TRUE;
	}

	#endif

//_LSEEK:

	if (!stream->common.no_lseek)
	{
		off = lseek(fd, 0, SEEK_CUR);
		if (off >= 0)
		{
			end = lseek(fd, 0, SEEK_END);
			if (end >= 0)
			{
				*len = (int)(end - off);

				off = lseek(fd, off, SEEK_SET);
				if (off >= 0)
					return FALSE;
			}
		}

		stream->common.no_lseek = TRUE;
	}

	//fprintf(stderr, "STREAM_get_readable: lseek: %d\n", *len);
	return TRUE;
}

bool STREAM_default_eof(STREAM *stream)
{
	int fd;
	int ilen;

	fd = STREAM_handle(stream);
	if (fd < 0)
		return TRUE;

	if (STREAM_is_blocking(stream) && !stream->common.available_now)
		wait_for_fd_ready_to_read(STREAM_handle(stream));

	if (!STREAM_get_readable(stream, &ilen))
		return (ilen == 0);
	
	return FALSE; // Unable to get the remaining size.
}

// STREAM_open *MUST* initialize completely the stream structure

void STREAM_open(STREAM *stream, const char *path, int mode)
{
	STREAM_CLASS *sclass;
	int fd;

	stream->type = NULL;

	if (mode & GB_ST_PIPE)
		sclass = &STREAM_pipe;
	else if (mode & GB_ST_MEMORY)
		sclass = &STREAM_memory;
	else if (mode & GB_ST_STRING)
		sclass = &STREAM_string;
	else if (mode & GB_ST_LOCK)
		sclass = &STREAM_lock;
	else if (mode & GB_ST_NULL)
		sclass = &STREAM_null;
	else
	{
		// ".99" is used for opening a file descriptor in direct mode

		if (FILE_is_relative(path) && !(((mode & GB_ST_BUFFERED) == 0) && path[0] == '.' && isdigit(path[1])))
		{
			ARCHIVE *arch = NULL;
			const char *tpath = path;

			/*ARCHIVE *arch = NULL;

			if (strncmp(path, "../", 3))
				ARCHIVE_get_current(&arch);
			else if (!EXEC_arch)
				path += 3;

			if ((arch && arch->name) || EXEC_arch) // || !FILE_exist_real(path)) - Why that test ?
			{
				sclass = &STREAM_arch;
				goto _OPEN;
			}*/

			if ((mode & GB_ST_ACCESS) != GB_ST_READ || mode & GB_ST_PIPE)
				THROW(E_ACCESS);

			if (!ARCHIVE_find_from_path(&arch, &tpath))
			{
				sclass = &STREAM_arch;
				goto __OPEN;
			}

			path = tpath;
		}

		if (mode & GB_ST_BUFFERED)
			sclass = &STREAM_buffer;
		else
			sclass = &STREAM_direct;
	}

__OPEN:

	stream->common.mode = mode;
	stream->common.swap = FALSE;
	stream->common.eol = 0;
	stream->common.eof = FALSE;
	stream->common.extra = NULL;
	stream->common.no_fionread = FALSE;
	stream->common.no_lseek = FALSE;
	stream->common.standard = FALSE;
	stream->common.blocking = TRUE;
	stream->common.available_now = FALSE;
	stream->common.redirected = FALSE;
	stream->common.no_read_ahead = FALSE;
	stream->common.null_terminated = FALSE;
	stream->common.check_read = FALSE;

	if ((*(sclass->open))(stream, path, mode, NULL))
		THROW_SYSTEM(errno, path);

	stream->type = sclass;

	fd = STREAM_handle(stream);
	if (fd >= 0)
		fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | O_CLOEXEC);

	#if DEBUG_STREAM
	_tag++;
	stream->common.tag = _tag;
	_nopen++;
	fprintf(stderr, "Open %p [%d] (%d)\n", stream, _tag, _nopen);
	#endif
}

static void release_buffer(STREAM_EXTRA *extra)
{
	if (extra && extra->buffer)
	{
		#if DEBUG_STREAM
		fprintf(stderr, "Stream %p [%d]: Free buffer\n", stream, stream->common.tag);
		#endif
		FREE(&extra->buffer);
		extra->buffer_pos = 0;
		extra->buffer_len = 0;
	}
}

static void release_unread(STREAM_EXTRA *extra)
{
	if (extra && extra->unread)
	{
		FREE(&extra->unread);
		extra->unread_pos = 0;
		extra->unread_len = 0;
	}
}

void STREAM_release(STREAM *stream)
{
	STREAM_EXTRA *extra = EXTRA(stream);
	
	STREAM_cancel(stream);
	release_buffer(extra);
	release_unread(extra);
	
	if (extra)
	{
		ARRAY_delete(&extra->read_objects);
		HASH_TABLE_delete(&extra->write_objects);
		IFREE(extra);
		stream->common.extra = NULL;
	}
}

static void stop_watching(STREAM *stream, int mode)
{
	int fd = STREAM_handle(stream);
	if (fd >= 0)
		GB_Watch(fd, mode, NULL, 0);
}

void STREAM_close(STREAM *stream)
{
	STREAM_release(stream);

	if (!stream->type)
		return;

	stop_watching(stream, GB_WATCH_NONE);

	if (!stream->common.standard)
	{
	  if ((*(stream->type->close))(stream))
		{
			if (errno != EBADF && errno != EINPROGRESS && errno != EAGAIN)
				THROW_SYSTEM(errno, "");
		}
	}

	stream->type = NULL;

	#if DEBUG_STREAM
	_nopen--;
	fprintf(stderr, "Close %p [%d] (%d)\n", stream, stream->common.tag, _nopen);
	#endif
}


void STREAM_flush(STREAM *stream)
{
	STREAM_end(stream);

	if (!stream->type)
		THROW(E_CLOSED);

	(*(stream->type->flush))(stream);
}

static int read_unread(STREAM_EXTRA *extra, void *addr, int len)
{
	int l = extra->unread_len - extra->unread_pos;
	if (l > len)
		l = len;
	
	if (l > 0)
	{
		memcpy(addr, extra->unread + extra->unread_pos, l);
		extra->unread_pos += l;
		if (extra->unread_pos >= extra->unread_len)
			release_unread(extra);
		return l;
	}
	else
		return 0;
}

static int read_buffer(STREAM_EXTRA *extra, void *addr, int len)
{
	int l = extra->buffer_len - extra->buffer_pos;
	if (l > len)
		l = len;
	if (l > 0)
	{
		memcpy(addr, extra->buffer + extra->buffer_pos, l);
		extra->buffer_pos += l;
		return l;
	}
	else
		return 0;
}

int STREAM_read(STREAM *stream, void *addr, int len)
{
	int eff = 0;
	int n;
	
	if (STREAM_is_closed(stream))
		THROW(E_CLOSED);

	if (len <= 0)
		return 0;

	if (EXTRA(stream))
	{
		if (EXTRA(stream)->unread)
		{
			eff = read_unread(EXTRA(stream), addr, len);
			addr += eff;
			len -= eff;
			
			return eff;
		}
		
		if (len > 0 && EXTRA(stream)->buffer)
		{
			eff = read_buffer(EXTRA(stream), addr, len);
			addr += eff;
			len -= eff;
		}
	}

	while (len > 0)
	{
		n = (*(stream->type->read))(stream, addr, len);
		
		if ((n <= 0) && errno != EINTR)
		{
			stop_watching(stream, GB_WATCH_READ);
			
			if (n == 0)
				THROW(E_EOF);

			switch(errno)
			{
				case 0:
				case EAGAIN:
					THROW(E_EOF);
				case EIO:
					THROW(E_READ);
				default:
					THROW_SYSTEM(errno, NULL);
			}
		}

		eff += n;
		addr += n;
		len -= n;
	}
	
	return eff;
}

void STREAM_peek(STREAM *stream, void *addr, int len)
{
	STREAM_EXTRA *extra = ENSURE_EXTRA(stream);
	
	release_unread(extra);
	
	STREAM_read(stream, addr, len);
	
	ALLOC(&extra->unread, len);
	extra->unread_len = len;
	extra->unread_pos = 0;
	memcpy(extra->unread, addr, len);
}

#if 0
char STREAM_getchar(STREAM *stream)
{
	char c = 0;
	bool ret;

	if (!stream->type)
		THROW(E_CLOSED);

	if (stream->common.buffer && stream->common.buffer_pos < stream->common.buffer_len)
		return stream->common.buffer[stream->common.buffer_pos++];

	for(;;)
	{
		if (stream->type->getchar)
			ret = (*(stream->type->getchar))(stream, &c);
		else
			ret = (*(stream->type->read))(stream, &c, 1);

		if (ret == 1)
			break;

		if (errno == EINTR)
			continue;

		stop_watching(stream, GB_WATCH_READ);

		switch(errno)
		{
			case 0:
			case EAGAIN:
				THROW(E_EOF);
			case EIO:
				THROW(E_READ);
			default:
				THROW_SYSTEM(errno, NULL);
		}
	}

	return c;
}
#endif


int STREAM_read_max(STREAM *stream, void *addr, int len)
{
	int eff = 0;
	int n;
	int flags, handle;
	int save_errno;

	if (!stream->type)
		THROW(E_CLOSED);
	
	if (len <= 0)
		return 0;

	if (EXTRA(stream))
	{
		if (EXTRA(stream)->unread)
		{
			eff = read_unread(EXTRA(stream), addr, len);
			addr += eff;
			len -= eff;
			
			return eff;
		}
		
		if (len > 0 && EXTRA(stream)->buffer)
		{
			eff = read_buffer(EXTRA(stream), addr, len);
			addr += eff;
			len -= eff;
		}
	}

	while (len > 0)
	{
		if (stream->common.available_now)
		{
			n = (*(stream->type->read))(stream, addr, len);
		}
		else
		{
			handle = STREAM_handle(stream);
			flags = fcntl(handle, F_GETFL);
			if ((flags & O_NONBLOCK) == 0)
			{
				wait_for_fd_ready_to_read(handle);
				fcntl(handle, F_SETFL, flags | O_NONBLOCK);
			}

			errno = 0;
			n = (*(stream->type->read))(stream, addr, len);
			save_errno = errno;

			if ((flags & O_NONBLOCK) == 0)
				fcntl(handle, F_SETFL, flags);

			errno = save_errno;
		}
		
		if (n > 0)
		{
			eff += n;
			break;
		}

		if (n <= 0 && errno != EINTR)
		{
			if (n == 0 || errno == 0)
			{
				stop_watching(stream, GB_WATCH_READ);
				return eff;
			}
			
			switch(errno)
			{
				case EAGAIN:
				case EIO:
					return eff;
				default:
					THROW_SYSTEM(errno, NULL);
			}
		}

		addr += n;
		len -= n;
	}

	return eff;
}


void STREAM_write(STREAM *stream, void *addr, int len)
{
	int n;
	
	if (STREAM_is_closed_for_writing(stream))
		THROW(E_CLOSED);

	if (len <= 0)
		return;
	
	if (stream->common.redirected)
		stream = EXTRA(stream)->redirect;

	do
	{
		n = ((*(stream->type->write))(stream, addr, len));
		
		if (n <= 0 && errno != EINTR)
		{
			switch(errno)
			{
				case 0:
				case EIO:
					THROW(E_WRITE);
				default:
					THROW_SYSTEM(errno, NULL);
			}
		}
		
		addr += n;
		len -= n;
	
		if (STREAM_is_closed_for_writing(stream))
			THROW(E_CLOSED);
	}
	while (len > 0);
}


void STREAM_write_zeros(STREAM *stream, int len)
{
	static const char buffer[32] = { 0 };
	int lenw;

	while (len > 0)
	{
		lenw = Min(len, sizeof(buffer));
		STREAM_write(stream, (void *)buffer, lenw);
		len -= lenw;
	}
}


void STREAM_write_eol(STREAM *stream)
{
	if (STREAM_is_closed_for_writing(stream))
		THROW(E_CLOSED);

	switch(stream->common.eol)
	{
		case ST_EOL_UNIX: STREAM_write(stream, "\n", 1); break;
		case ST_EOL_WINDOWS: STREAM_write(stream, "\r\n", 2); break;
		case ST_EOL_MAC: STREAM_write(stream, "\r", 1); break;
	}
}


int64_t STREAM_tell(STREAM *stream)
{
	int64_t pos;

	if (STREAM_is_closed(stream))
		THROW(E_CLOSED);

	if (stream->type->tell(stream, &pos))
		/*THROW(E_SEEK, ERROR_get());*/
		THROW_SYSTEM(errno, NULL);

	return pos;
}


void STREAM_seek(STREAM *stream, int64_t pos, int whence)
{
	if (STREAM_is_closed(stream))
		THROW(E_CLOSED);

	if (stream->type->seek(stream, pos, whence))
	{
		switch(errno)
		{
			case EINVAL:
				THROW(E_ARG);
			default:
				THROW_SYSTEM(errno, NULL);
		}
	}

	release_buffer(EXTRA(stream));
}

static int fill_buffer(STREAM *stream, char *addr, bool do_not_wait_ready)
{
	int n;
	int flags, fd;
	int len;
	int eff = 0;

	fd = STREAM_handle(stream);
	len = STREAM_BUFFER_SIZE;

	while (len > 0)
	{
		if (stream->common.available_now)
			n = (*(stream->type->read))(stream, addr, len);
		else
		{
			if (!do_not_wait_ready)
			{
				wait_for_fd_ready_to_read(fd);
				do_not_wait_ready = TRUE;
			}

			flags = fcntl(fd, F_GETFL);
			if ((flags & O_NONBLOCK) == 0)
				fcntl(fd, F_SETFL, flags | O_NONBLOCK);

			n = (*(stream->type->read))(stream, addr, len);

			if ((flags & O_NONBLOCK) == 0)
			{
				int save_errno = errno;
				fcntl(fd, F_SETFL, flags);
				errno = save_errno;
			}
		}

		if (n == 0)
			return eff;
		
		if (n > 0)
			eff += n;
			
		if (n < 0 && errno != EINTR)
		{
			switch(errno)
			{
				case EAGAIN:
				case EINPROGRESS:
				case EIO:
					return eff;
				default:
					THROW_SYSTEM(errno, NULL);
			}
		}
		
		addr += n;
		len -= n;
	}
	
	return eff;
}

bool STREAM_read_ahead(STREAM *stream)
{
	int eff;
	STREAM_EXTRA *extra = EXTRA(stream);
	
	if (stream->common.no_read_ahead)
		return FALSE;

	if (extra && extra->buffer && extra->buffer_pos < extra->buffer_len)
		return FALSE;

	extra = ENSURE_EXTRA(stream);
	
	if (!extra->buffer)
		ALLOC(&extra->buffer, STREAM_BUFFER_SIZE);

	eff = fill_buffer(stream, extra->buffer, TRUE);
	
	extra->buffer_pos = 0;
	extra->buffer_len = eff;

	if (eff == 0)
	{
		stream->common.eof = TRUE;
		return TRUE;
	}
	else
	{
		stream->common.eof = FALSE;
		return FALSE;
	}
}


static char *input(STREAM *stream, bool line, char *escape)
{
	unsigned char mode;
	int len = 0;
	int start;
	unsigned char c = 0, lc = 0;
	void *test;
	char *buffer;
	int buffer_len;
	int buffer_pos;
	char *addr;
	char ec;
	bool inside_escape = FALSE;
	STREAM_EXTRA *extra;

	addr = NULL;

	mode = 2;
	ec = escape ? *escape : 0;

	if (!line)
		test = &&__TEST_INPUT;
	else
	{
		switch(stream->common.eol)
		{
			case GB_EOL_WINDOWS: test = &&__TEST_WINDOWS; break;
			case GB_EOL_MAC: test = &&__TEST_MAC; break;
			default: test = NULL; mode = ec ? 1 : 0; break;
		}
	}

	stream->common.eof = FALSE;

	if (!STREAM_is_blocking(stream) && STREAM_eof(stream))
		THROW(E_EOF);

	extra = ENSURE_EXTRA(stream);
	
	buffer = extra->buffer;
	buffer_len = extra->buffer_len;
	buffer_pos = extra->buffer_pos;

	if (!buffer)
	{
		#if DEBUG_STREAM
		fprintf(stderr, "Stream %p [%d]: Alloc buffer\n", stream, stream->common.tag);
		#endif
		ALLOC(&buffer, STREAM_BUFFER_SIZE);
		buffer_pos = 0;
		buffer_len = 0;
	}

	start = buffer_pos;

	for(;;)
	{
		if (mode == 0)
		{
			while (buffer_pos < buffer_len)
			{
				c = buffer[buffer_pos++]; //STREAM_getchar(stream);

				if (c == '\n')
				{
					len = buffer_pos - start - 1;
					if (buffer_pos)
						lc = buffer[buffer_pos - 1];
					if (lc == '\r')
						len--;
					goto __FINISH;
				}
			}

			len = buffer_len - start;
		}
		else if (mode == 1)
		{
			while (buffer_pos < buffer_len)
			{
				c = buffer[buffer_pos++]; //STREAM_getchar(stream);

				if (c == ec)
					inside_escape = !inside_escape;
				else if (!inside_escape)
				{
					if (c == '\n')
					{
						len = buffer_pos - start - 1;
						if (buffer_pos)
							lc = buffer[buffer_pos - 1];
						if (lc == '\r')
							len--;
						goto __FINISH;
					}
				}
			}

			len = buffer_pos - start;
		}
		else
		{
			while (buffer_pos < buffer_len)
			{
				c = buffer[buffer_pos++]; //STREAM_getchar(stream);

				if (ec && c == ec)
				{
					inside_escape = !inside_escape;
					continue;
				}

				if (inside_escape)
					continue;

				goto *test;

			__TEST_INPUT:

				if (c <= ' ')
				{
					len = buffer_pos - start - 1;
					goto __FINISH;
				}
				else
					continue;

			__TEST_MAC:

				if (c == '\r')
				{
					len = buffer_pos - start - 1;
					goto __FINISH;
				}
				else
					continue;

			__TEST_WINDOWS:

				if ((lc == '\r') && (c == '\n'))
				{
					len = buffer_pos - start - 2;
					goto __FINISH;
				}
				else
				{
					lc = c;
					continue;
				}
			}

			len = buffer_pos - start;
		}

		lc = c;

		if (len)
		{
			if (!addr)
				addr = STRING_new(buffer + start, len);
			else
				addr = STRING_add(addr, buffer + start, len);
			len = 0;
		}

		extra->buffer = buffer;
		extra->buffer_pos = buffer_pos;
		extra->buffer_len = buffer_len;

		buffer_pos = 0;
		buffer_len = fill_buffer(stream, buffer, FALSE);

		if (!buffer_len)
		{
			stream->common.eof = TRUE;
			break;
		}

		start = 0;
	}

__FINISH:

	if (len > 0)
	{
		if (!addr)
			addr = STRING_new(buffer + start, len);
		else
			addr = STRING_add(addr, buffer + start, len);
	}
	else if (len < 0)
		addr = STRING_extend(addr, STRING_length(addr) + len);

	extra->buffer = buffer;
	extra->buffer_pos = buffer_pos;
	extra->buffer_len = buffer_len;

	return addr;
}


char *STREAM_line_input(STREAM *stream, char *escape)
{
	return input(stream, TRUE, escape);
}


char *STREAM_input(STREAM *stream)
{
	return input(stream, FALSE, NULL);
}


static int read_length(STREAM *stream)
{
	union
	{
		unsigned char _data[4];
		short _short;
		int _int;
	}
	buffer;

	int len = 0;

	STREAM_read(stream, buffer._data, 1);

	switch (buffer._data[0] >> 6)
	{
		case 0:
		case 1:
			len = buffer._data[0];
			break;

		case 2:
			STREAM_read(stream, &buffer._data[1], 1);
			buffer._data[0] &= 0x3F;

			if (!EXEC_big_endian)
				SWAP_short(&buffer._short);

			len = buffer._short;
			break;

		case 3:
			STREAM_read(stream, &buffer._data[1], 3);
			buffer._data[0] &= 0x3F;

			if (!EXEC_big_endian)
				SWAP_int(&buffer._int);

			len = buffer._int;
			break;
	}

	return len;
}

static STREAM *enter_temp_stream(STREAM *stream)
{
	if (stream != CSTREAM_TO_STREAM(_temp_stream))
	{
		_temp_save = stream;
		_temp_level = 0;
		
		OBJECT_UNREF(_temp_stream);
		_temp_stream = OBJECT_new(CLASS_File, NULL, NULL);
		STREAM_open(CSTREAM_TO_STREAM(_temp_stream), NULL, GB_ST_STRING | GB_ST_WRITE);
	}

	_temp_level++;

	return CSTREAM_TO_STREAM(_temp_stream);
}

static STREAM *leave_temp_stream(void)
{
	STREAM *stream = CSTREAM_TO_STREAM(_temp_stream);
	
	_temp_level--;
	if (_temp_level > 0)
		return stream;

	STREAM_write(_temp_save, stream->string.buffer, STRING_length(stream->string.buffer));
	STREAM_close(stream);
	OBJECT_UNREF(_temp_stream);
	return _temp_save;
}

static void read_structure(STREAM *stream, CLASS *class, char *base);

static void read_value_ctype(STREAM *stream, CLASS *class, CTYPE ctype, void *addr)
{
	TYPE type;
	VALUE temp;

	type = (TYPE)ctype.id;
	if (type == T_OBJECT && ctype.value >= 0)
	{
		class = class->load->class_ref[ctype.value];
		if (CLASS_is_struct(class))
		{
			CSTRUCT *structure = (CSTRUCT *)OBJECT_create(class, NULL, NULL, 0);
			OBJECT_REF(structure);
			read_structure(stream, class, (char *)structure + sizeof(CSTRUCT));
			*((void **)addr) = structure;
			return;
		}
	}
	else if (type == TC_STRUCT)
	{
		class = class->load->class_ref[ctype.value];
		read_structure(stream, class, addr);
		return;
	}

	STREAM_read_type(stream, type, &temp);
	VALUE_class_write(class, &temp, addr, ctype);
}

static CLASS *read_class(STREAM *stream, TYPE type)
{
	char *name = COMMON_buffer;
	unsigned char len;
	CLASS *class;
	
	STREAM_read(stream, &len, 1);
	STREAM_read(stream, name, len);
	name[len] = 0;

	class = CLASS_get(name);
	if (!class)
		THROW_SERIAL();
	
	if (TYPE_is_pure_object(type))
	{
		if ((CLASS *)type != class)
			THROW_SERIAL();
	}
	
	return class;
}

static void read_structure(STREAM *stream, CLASS *class, char *base)
{
	int i, n;
	CLASS_DESC *desc;
	char *addr;
	CTYPE ctype;

	for (n = 0; n < class->n_desc; n++)
	{
		desc = class->table[n].desc;
		ctype = desc->variable.ctype;
		addr = base + desc->variable.offset;

		if (ctype.id == TC_STRUCT)
		{
			read_structure(stream, class->load->class_ref[ctype.value], addr);
		}
		else if (ctype.id == TC_ARRAY)
		{
			CLASS_ARRAY *adesc = class->load->array[ctype.value];
			int size = CLASS_sizeof_ctype(class, adesc->ctype);

			for (i = 0; i < CARRAY_get_static_count(adesc); i++)
			{
				read_value_ctype(stream, desc->variable.class, adesc->ctype, addr);
				addr += size;
			}
		}
		else
		{
			read_value_ctype(stream, desc->variable.class, ctype, addr);
		}
	}
}

static void register_read_object(STREAM *stream, void *object)
{
	STREAM_EXTRA *extra = ENSURE_EXTRA(stream);
	
	if (!extra->read_objects)
		ARRAY_create(&extra->read_objects);
	
	//fprintf(stderr, "register_read_object: %p -> %d\n", object, ARRAY_count(extra->read_objects));
	*(void **)ARRAY_add(&extra->read_objects) = object;
}

static void error_STREAM_read_type(void *object)
{
	OBJECT_UNREF(object);
}

void STREAM_read_type(STREAM *stream, TYPE type, VALUE *value)
{
	bool variant;
	int len;

	union
	{
		unsigned char _byte;
		short _short;
		int _int;
		float _single;
		unsigned char _data[4];
	}
	buffer;

	variant = (type == T_VARIANT);

	if (variant || TYPE_is_object(type))
	{
		if (TYPE_is_pure_object(type) && CLASS_is_struct((CLASS *)type))
		{
			CLASS *class = (CLASS *)type;
			CSTRUCT *structure = (CSTRUCT *)OBJECT_create(class, NULL, NULL, 0);

			read_structure(stream, class, (char *)structure + sizeof(CSTRUCT));

			value->_object.class = class;
			value->_object.object = structure;

			return;
		}
		
		STREAM_read(stream, &buffer._byte, 1);

		if (buffer._byte == 0)
		{
			value->type = T_VARIANT;
			value->_variant.vtype = T_NULL;
			return;
		}
		
		if (buffer._byte == '@')
		{
			void *object;
			STREAM_EXTRA *extra = EXTRA(stream);
			int id;
			
			if (!extra)
				THROW(E_SERIAL);
			
			STREAM_read(stream, &id, sizeof(int));
			id--;
			if (id < 0 || id > ARRAY_count(extra->read_objects))
				THROW(E_SERIAL);
			
			object = extra->read_objects[id];
			//fprintf(stderr, "find: %d -> %p\n", id, object);
			value->_object.object = object;
			value->type = (TYPE)OBJECT_class(object);
			
			if (value->type != type)
			{
				OBJECT_REF(object);
				VALUE_convert(value, type);
				UNBORROW(value);
			}
			
			return;
		}
		
		if (buffer._byte == 'A' || buffer._byte == 'a')
		{
			CLASS *class;
			CARRAY *array;
			int size, i;
			VALUE temp;
			void *data;
			TYPE atype = T_VOID;

			if (buffer._byte == 'a' || variant)
			{
				class = read_class(stream, type);
				
				atype = class->array_type;
			}
			else if (TYPE_is_pure_object(type))
			{
				atype = ((CLASS *)type)->array_type;
			}
			else if (TYPE_is_object(type))
			{
				atype = T_VARIANT;
			}

			if (TYPE_is_void(atype))
				THROW_SERIAL();
			
			if (TYPE_is_pure_object(atype))
				CLASS_load((CLASS *)atype);
			
			STREAM_read(stream, &buffer._byte, 1); // Compatibility with old format
			if (atype == T_VARIANT)
				atype = (TYPE)buffer._byte;
			
			size = read_length(stream);

			GB_ArrayNew((GB_ARRAY *)&array, atype, size);
			register_read_object(stream, array);
			
			for (i = 0; i < size; i++)
			{
				data = CARRAY_get_data(array, i);
				STREAM_read_type(stream, atype, &temp);
				VALUE_write(&temp, data, atype);
				//RELEASE(&temp);
			}

			value->type = (TYPE)OBJECT_class(array);
			value->_object.object = array;
			
			if (value->type != type)
			{
				OBJECT_REF(array);
				VALUE_convert(value, type);
				UNBORROW(value);
			}
			return;
		}

		if (buffer._byte == 'c' || buffer._byte == 'C')
		{
			GB_COLLECTION col;
			int size, i;
			VALUE temp;
			char *key;
			char tkey[32];
			int len;

			size = read_length(stream);

			GB_CollectionNew(&col, buffer._byte == 'c');
			register_read_object(stream, col);
			
			for (i = 0; i < size; i++)
			{
				len = read_length(stream);
				if (len < sizeof(tkey))
					key = tkey;
				else
					key = STRING_new(NULL, len);
				STREAM_read(stream, key, len);
				STREAM_read_type(stream, T_VARIANT, &temp);
				GB_CollectionSet(col, key, len, (GB_VARIANT *)&temp);
				//RELEASE(&temp);
				if (len >= 32)
					STRING_free_real(key);
			}

			value->type = (TYPE)OBJECT_class(col);
			value->_object.object = col;
			
			if (value->type != type)
			{
				OBJECT_REF(col);
				VALUE_convert(value, type);
				UNBORROW(value);
			}
			return;
		}
		
		if (buffer._byte == 'O' || buffer._byte == 'o')
		{
			CLASS *class = NULL;
			void *object;
			void *cstream;
			
			if (buffer._byte == 'o' || variant)
				class = read_class(stream, type);
			else
				class = (CLASS *)type;
			
			object = OBJECT_REF(OBJECT_create(class, NULL, NULL, 0));
			register_read_object(stream, object);
			
			cstream = CSTREAM_FROM_STREAM(stream);
			
			STACK_check(1);
			PUSH_OBJECT(OBJECT_class(cstream), cstream);
			
			ON_ERROR_1(error_STREAM_read_type, object)
			{
				if (EXEC_special(SPEC_READ, class, object, 1, TRUE))
					THROW_SERIAL();
			}
			END_ERROR
			
			OBJECT_UNREF_KEEP(object);

			value->type = (TYPE)class;
			value->_object.object = object;
			if (value->type != type)
			{
				OBJECT_REF(object);
				VALUE_convert(value, type);
				UNBORROW(value);
			}
			return;
		}

		if (variant)
			type = buffer._byte;
	}

	value->type = type;

	switch (type)
	{
		case T_BOOLEAN:

			STREAM_read(stream, &buffer._byte, 1);
			value->_integer.value = (buffer._byte != 0) ? (-1) : 0;
			break;

		case T_BYTE:

			STREAM_read(stream, &buffer._byte, 1);
			value->_integer.value = buffer._byte;
			break;

		case T_SHORT:

			STREAM_read(stream, &buffer._short, sizeof(short));
			if (stream->common.swap)
				SWAP_short(&buffer._short);
			value->_integer.value = buffer._short;
			break;

		case T_INTEGER:

			STREAM_read(stream, &value->_integer.value, sizeof(int));
			if (stream->common.swap)
				SWAP_int(&value->_integer.value);
			break;

		case T_LONG:

			STREAM_read(stream, &value->_long.value, sizeof(int64_t));
			if (stream->common.swap)
				SWAP_int64(&value->_long.value);
			break;

		case T_POINTER:

			STREAM_read(stream, &value->_pointer.value, sizeof(void *));
			if (stream->common.swap)
				SWAP_pointer(&value->_pointer.value);
			break;

		case T_SINGLE:

			STREAM_read(stream, &buffer._single, sizeof(float));
			if (stream->common.swap)
				SWAP_float(&buffer._single);
			value->_single.value = buffer._single;
			break;

		case T_FLOAT:

			STREAM_read(stream, &value->_float.value, sizeof(double));
			if (stream->common.swap)
				SWAP_double(&value->_float.value);
			break;

		case T_DATE:

			STREAM_read(stream, &value->_date.date, sizeof(int));
			STREAM_read(stream, &value->_date.time, sizeof(int));
			if (stream->common.swap)
			{
				SWAP_int(&value->_date.date);
				SWAP_int(&value->_date.time);
			}
			break;

		case T_CSTRING:
			value->type = T_STRING;
			// continue

		case T_STRING:

			if (stream->common.null_terminated)
			{
				if (stream->type == &STREAM_memory)
				{
					ssize_t slen;
					if (CHECK_strlen(stream->memory.addr + stream->memory.pos, &slen))
						THROW(E_READ);
					len = (int)slen;
				}
				else
				{
					len = -1;
				}
			}
			else
				len = read_length(stream);

			if (len > 0)
			{
				STRING_new_temp_value(value, NULL, len);
				STREAM_read(stream, value->_string.addr, len);
			}
			else if (len < 0)
			{
				char *str = NULL;
				
				for(len = 0;; len++)
				{
					STREAM_read(stream, &buffer._byte, 1);
					if (buffer._byte == 0)
						break;
					str = STRING_add_char(str, buffer._byte);
				}
				STRING_free_later(str);
				
				value->_string.addr = str;
				value->_string.len = len;
				value->_string.start = 0;
				value->type = T_STRING;
			}
			else
			{
				STRING_void_value(value);
			}

			if (stream->common.null_terminated)
				STREAM_seek(stream, SEEK_CUR, 1);
				
			break;

		case T_NULL:

			break;

		default:

			THROW_SERIAL();
	}

	if (variant)
		VALUE_convert_variant(value);
}

static void write_length(STREAM *stream, int len)
{
	union
	{
		unsigned char _byte;
		short _short;
		int _int;
	}
	buffer;

	if (len < 0x80)
	{
		buffer._byte = (unsigned char)len;
		STREAM_write(stream, &buffer._byte, 1);
	}
	else if (len < 0x4000)
	{
		buffer._short = (short)len | 0x8000;
		if (!EXEC_big_endian)
			SWAP_short(&buffer._short);
		STREAM_write(stream, &buffer._short, sizeof(short));
	}
	else
	{
		buffer._int = len | 0xC0000000;
		if (!EXEC_big_endian)
			SWAP_int(&buffer._int);
		STREAM_write(stream, &buffer._int, sizeof(int));
	}
}

void STREAM_write_type(STREAM *stream, TYPE type, VALUE *value)
{
	union
	{
		unsigned char _byte;
		short _short;
		int _int;
		int64_t _long;
		float _single;
		double _float;
		unsigned char _data[8];
		void *_pointer;
	}
	buffer;

	CLASS *class;
	void *object;
	void *structure;
	bool variant;
	STREAM_EXTRA *extra;
	int *pid;

	if (type == T_VARIANT)
	{
		variant = TRUE;
		VARIANT_undo(value);
		type = value->type;
	}
	else
		variant = FALSE;

	if (!TYPE_is_object(type))
	{
		if (variant)
		{
			buffer._byte = (unsigned char)type;
			STREAM_write(stream, &buffer._byte, 1);
		}

		switch (type)
		{
			case T_BOOLEAN:

				buffer._byte = value->_integer.value ? 0xFF : 0;
				STREAM_write(stream, &buffer._byte, 1);
				break;

			case T_BYTE:

				buffer._byte = (unsigned char)value->_integer.value;
				STREAM_write(stream, &buffer._byte, 1);
				break;

			case T_SHORT:

				buffer._short = (short)value->_integer.value;
				if (stream->common.swap)
					SWAP_short(&buffer._short);
				STREAM_write(stream, &buffer._short, sizeof(short));
				break;

			case T_INTEGER:

				buffer._int = value->_integer.value;
				if (stream->common.swap)
					SWAP_int(&buffer._int);
				STREAM_write(stream, &buffer._int, sizeof(int));
				break;

			case T_LONG:

				buffer._long = value->_long.value;
				if (stream->common.swap)
					SWAP_int64(&buffer._long);
				STREAM_write(stream, &buffer._long, sizeof(int64_t));
				break;

			case T_POINTER:

				buffer._pointer = value->_pointer.value;
				if (stream->common.swap)
					SWAP_pointer(&buffer._pointer);
				STREAM_write(stream, &buffer._pointer, sizeof(void *));
				break;

			case T_SINGLE:

				buffer._single = value->_single.value;
				if (stream->common.swap)
					SWAP_float(&buffer._single);
				STREAM_write(stream, &buffer._single, sizeof(float));
				break;

			case T_FLOAT:

				buffer._float = value->_float.value;
				if (stream->common.swap)
					SWAP_double(&buffer._float);
				STREAM_write(stream, &buffer._float, sizeof(double));
				break;

			case T_DATE:

				buffer._int = value->_date.date;
				if (stream->common.swap)
					SWAP_int(&buffer._int);
				STREAM_write(stream, &buffer._int, sizeof(int));

				buffer._int = value->_date.time;
				if (stream->common.swap)
					SWAP_int(&buffer._int);
				STREAM_write(stream, &buffer._int, sizeof(int));

				break;

			case T_STRING:
			case T_CSTRING:

				if (stream->common.null_terminated)
				{
					STREAM_write(stream, value->_string.addr + value->_string.start, value->_string.len);
					buffer._byte = 0;
					STREAM_write(stream, &buffer._byte, 1);
				}
				else
				{
					write_length(stream, value->_string.len);
					STREAM_write(stream, value->_string.addr + value->_string.start, value->_string.len);
				}
				break;

			case T_NULL:

				break;

			default:

				THROW_SERIAL();
		}
		
		return;
	}

	object = value->_object.object;
	
	if (!object)
	{
		buffer._byte = 0;
		STREAM_write(stream, &buffer._byte, 1);
		return;
	}
	
	extra = ENSURE_EXTRA(stream);
	if (!extra->write_objects)
		HASH_TABLE_create(&extra->write_objects, sizeof(int), HF_NORMAL);
	
	pid = (int *)HASH_TABLE_lookup(extra->write_objects, (const char *)&object, sizeof(uintptr_t), FALSE);
	if (pid)
	{
		buffer._byte = '@';
		STREAM_write(stream, &buffer._byte, 1);
		buffer._int = *pid;
		STREAM_write(stream, &buffer._int, sizeof(int));
		return;
	}
	else
	{
		pid = HASH_TABLE_insert(extra->write_objects, (const char *)&object, sizeof(uintptr_t));
		*pid = HASH_TABLE_size(extra->write_objects);
	}
	
	class = OBJECT_class(object);
	
	if (class->special[SPEC_WRITE] != NO_SYMBOL)
	{
		CSTREAM *ob;
		
		if (variant || type == T_OBJECT)
		{
			char *name = class->name;
			int len = strlen(name);
			
			buffer._byte = 'o';
			STREAM_write(stream, &buffer._byte, 1);
			buffer._byte = (unsigned char)len;
			STREAM_write(stream, &buffer._byte, 1);
			
			STREAM_write(stream, name, len);
		}
		else
		{
			buffer._byte = 'O';
			STREAM_write(stream, &buffer._byte, 1);
		}
		
		ob = CSTREAM_FROM_STREAM(stream);
		STACK_check(1);
		PUSH_OBJECT(OBJECT_class(ob), ob);
		EXEC_special(SPEC_WRITE, class, object, 1, TRUE);
		
		return;
	}
	
	if (class->quick_array == CQA_ARRAY || class->is_array_of_struct)
	{
		CARRAY *array = (CARRAY *)object;
		VALUE temp;
		void *data;
		int i;

		if (OBJECT_is_locked((OBJECT *)array))
			THROW_SERIAL();
		OBJECT_lock((OBJECT *)array, TRUE);

		if (!array->ref)
		{
			if (variant || type == T_OBJECT)
			{
				char *name = class->name;
				int len = strlen(name);
				
				buffer._byte = 'a';
				STREAM_write(stream, &buffer._byte, 1);
				
				buffer._byte = (unsigned char)len;
				STREAM_write(stream, &buffer._byte, 1);
				STREAM_write(stream, name, len);
			}
			else
			{
				buffer._byte = 'A';
				STREAM_write(stream, &buffer._byte, 1);
			}
			
			if (TYPE_is_pure_object(array->type))
				buffer._byte = T_OBJECT;
			else
				buffer._byte = (uchar)array->type;
			STREAM_write(stream, &buffer._byte, 1); // Compatibility with old format
			
			write_length(stream, array->count);
		}

		if (class->is_array_of_struct)
		{
			for (i = 0; i < array->count; i++)
			{
				data = CARRAY_get_data(array, i);
				structure = CSTRUCT_create_static(array, (CLASS *)array->type, data);
				temp._object.class = OBJECT_class(structure);
				temp._object.object = structure;
				OBJECT_REF(structure);
				STREAM_write_type(stream, T_OBJECT, &temp);
				OBJECT_UNREF(structure);
			}
		}
		else
		{
			for (i = 0; i < array->count; i++)
			{
				data = CARRAY_get_data(array, i);
				VALUE_read(&temp, data, array->type);
				STREAM_write_type(stream, array->type, &temp);
			}
		}

		OBJECT_lock((OBJECT *)array, FALSE);
		return;
	}
	
	if (class->quick_array == CQA_COLLECTION)
	{
		CCOLLECTION *col = (CCOLLECTION *)object;
		GB_COLLECTION_ITER iter;
		char *key = NULL;
		int len;
		VALUE temp;

		if (OBJECT_is_locked((OBJECT *)col))
			THROW_SERIAL();
		OBJECT_lock((OBJECT *)col, TRUE);

		buffer._byte = col->mode ? 'c' : 'C';
		STREAM_write(stream, &buffer._byte, 1);

		write_length(stream, CCOLLECTION_get_count(col));

		GB_CollectionEnum(col, &iter, (GB_VARIANT *)&temp, NULL, NULL);
		while (!GB_CollectionEnum(col, &iter, (GB_VARIANT *)&temp, &key, &len))
		{
			write_length(stream, len);
			STREAM_write(stream, key, len);
			STREAM_write_type(stream, T_VARIANT, &temp);
		}

		OBJECT_lock((OBJECT *)col, FALSE);
		return;
	}
	
	if (class->is_struct)
	{
		CSTRUCT *structure = (CSTRUCT *)object;
		int i;
		CLASS_DESC *desc;
		VALUE temp;
		char *addr;

		stream = enter_temp_stream(stream);

		if (OBJECT_is_locked((OBJECT *)structure))
			THROW_SERIAL();
		OBJECT_lock((OBJECT *)structure, TRUE);

		for (i = 0; i < class->n_desc; i++)
		{
			desc = class->table[i].desc;

			if (structure->ref)
				addr = (char *)((CSTATICSTRUCT *)structure)->addr + desc->variable.offset;
			else
				addr = (char *)structure + sizeof(CSTRUCT) + desc->variable.offset;

			VALUE_class_read(desc->variable.class, &temp, (void *)addr, desc->variable.ctype, (void *)structure);
			//BORROW(&temp);
			STREAM_write_type(stream, temp.type, &temp);
			RELEASE(&temp);
		}

		stream = leave_temp_stream();

		OBJECT_lock((OBJECT *)structure, FALSE);
		return;
	}
	
	THROW_SERIAL();
}


void STREAM_load(const char *path, char **buffer, int *rlen)
{
	STREAM stream;
	int64_t len;

	STREAM_open(&stream, path, GB_ST_READ);
	STREAM_lof(&stream, &len);

	if (len >> 31)
		THROW(E_MEMORY);

	*rlen = len;

	ALLOC(buffer, *rlen);

	STREAM_read(&stream, *buffer, *rlen);
	STREAM_close(&stream);
}


bool STREAM_map(const char *path, char **paddr, int *plen)
{
	STREAM stream;
	int fd;
  struct stat info;
	void *addr;
	size_t len;
	bool ret = TRUE;

	STREAM_open(&stream, path, GB_ST_READ);

	if (stream.type == &STREAM_arch)
	{
		*paddr = (char *)stream.arch.arch->arch->addr + stream.arch.start;
		*plen = stream.arch.size;
		ret = FALSE;
		goto __RETURN;
	}

	fd = STREAM_handle(&stream);
	if (fd < 0)
		goto __RETURN;

  if (fstat(fd, &info) < 0)
    goto __RETURN;

  len = info.st_size;
  addr = mmap(NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);
  if (addr == MAP_FAILED)
    goto __RETURN;

	*paddr = addr;
	*plen = len;
	ret = FALSE;

__RETURN:

	STREAM_close(&stream);
	return ret;
}

void STREAM_unmap(char *addr, int len)
{
  if (addr && len > 0 && ARCHIVE_check_addr(addr))
  {
  	munmap(addr, len);
	}
}


int STREAM_handle(STREAM *stream)
{
	if (STREAM_is_closed(stream))
		THROW(E_CLOSED);

	if (stream->type->handle)
		return (*stream->type->handle)(stream);
	else
		return (-1);
}

int STREAM_lock_all_fd(int fd)
{
	int64_t pos;
	
	pos = lseek(fd, 0, SEEK_CUR);
	if (pos < 0)
		return errno;

	if (lseek(fd, 0, SEEK_SET) < 0)
		return errno;

	#ifdef F_TLOCK

		if (lockf(fd, F_TLOCK, 0))
			return errno;

	#else

		return ENOTSUP;

	#endif

	if (lseek(fd, pos, SEEK_SET) < 0)
		return errno;

	return 0;
}

bool STREAM_lock_all(STREAM *stream)
{
	int fd;
	int err;

	if (STREAM_is_closed(stream))
		THROW(E_CLOSED);

	fd = STREAM_handle(stream);
	if (fd < 0)
		return TRUE;
	
	err = STREAM_lock_all_fd(fd);
	
	if (err == 0)
		return FALSE;
	if (err == EAGAIN)
		return TRUE;

	THROW_SYSTEM(err, NULL);
	return TRUE;
}


bool STREAM_lof_safe(STREAM *stream, int64_t *len)
{
	int ilen;
	STREAM_EXTRA *extra;

	*len = 0;

	extra = EXTRA(stream);
	if (extra && extra->unread)
	{
		*len = extra->unread_len - extra->unread_pos;
		if (*len > 0)
			return FALSE;
	}

	if (stream->type->lof)
	{
		if (!(*(stream->type->lof))(stream, len))
			goto ADD_BUFFER;
	}

	if (STREAM_get_readable(stream, &ilen))
		return TRUE;
	
	*len = ilen;

ADD_BUFFER:

	if (extra)
	{
		if (extra->buffer)
			*len += extra->buffer_len - extra->buffer_pos;
	}
	
	return FALSE;
}


void STREAM_lof(STREAM *stream, int64_t *len)
{
	if (STREAM_is_closed(stream))
		THROW(E_CLOSED);

	if (STREAM_lof_safe(stream, len))
		THROW(E_USIZE);
}


bool STREAM_eof(STREAM *stream)
{
	STREAM_EXTRA *extra = EXTRA(stream);
	
	if (STREAM_is_closed(stream))
		THROW(E_CLOSED);

	if (extra)
	{
		if (extra->unread && extra->unread_pos < extra->unread_len)
			return FALSE;
		if (extra->buffer && extra->buffer_pos < extra->buffer_len)
			return FALSE;
	}
	
	if (stream->common.eof)
		return TRUE;

	if (stream->type->eof)
		return ((*(stream->type->eof))(stream));
	else
		return STREAM_default_eof(stream);
}


#if 0
int STREAM_read_direct(int fd, char *buffer, int len)
{
	ssize_t eff_read;
	ssize_t len_read;

	while (len > 0)
	{
		len_read = Min(len, MAX_IO);
		eff_read = read(fd, buffer, len_read);

		if (eff_read > 0)
		{
			STREAM_eff_read += eff_read;
			len -= eff_read;
			buffer += eff_read;
		}

		if (eff_read < len_read)
		{
			if (eff_read == 0)
				errno = 0;
			if (eff_read <= 0 && errno != EINTR)
				return TRUE;
		}
	}

	return FALSE;
}

int STREAM_write_direct(int fd, char *buffer, int len)
{
	ssize_t eff_write;
	ssize_t len_write;

	while (len > 0)
	{
		len_write = Min(len, MAX_IO);
		eff_write = write(fd, buffer, len_write);

		if (eff_write < len_write)
		{
			if (eff_write <= 0 && errno != EINTR)
				return TRUE;
		}

		len -= eff_write;
		buffer += eff_write;
	}

	return FALSE;
}
#endif

void STREAM_blocking(STREAM *stream, bool block)
{
	int fd = STREAM_handle(stream);

	if (fd < 0)
		return;

	stream->common.blocking = block;

	if (block)
		fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) & ~O_NONBLOCK);
	else
		fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}

void STREAM_check_blocking(STREAM *stream)
{
	int fd = STREAM_handle(stream);

	stream->common.blocking = (fd < 0) ? TRUE : ((fcntl(fd, F_GETFL) & O_NONBLOCK) == 0);
}

void STREAM_cancel(STREAM *stream)
{
	if (!stream->common.redirected)
		return;

	STREAM_close(EXTRA(stream)->redirect);
	FREE(&EXTRA(stream)->redirect);
	stream->common.redirected = FALSE;
}

void STREAM_begin(STREAM *stream)
{
	STREAM_cancel(stream);

	if (!stream->common.redirected)
	{
		STREAM_EXTRA *extra = ENSURE_EXTRA(stream);
		ALLOC_ZERO(&extra->redirect, sizeof(STREAM));
		STREAM_open(extra->redirect, NULL, GB_ST_STRING | GB_ST_WRITE);
	}

	stream->common.redirected = TRUE;
}

void STREAM_end(STREAM *stream)
{
	STREAM_EXTRA *extra = EXTRA(stream);
	
	if (!stream->common.redirected)
		return;

	stream->common.redirected = FALSE;
	STREAM_write(stream, extra->redirect->string.buffer, STRING_length(extra->redirect->string.buffer));
	stream->common.redirected = TRUE;
	STREAM_cancel(stream);
}

