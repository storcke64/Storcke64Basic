/***************************************************************************

  gbx_stream.h

  (c) 2000-2009 Benoît Minisini <gambas@users.sourceforge.net>

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#ifndef __GBX_STREAM_H
#define __GBX_STREAM_H

#include "gbx_value.h"
#include "gbx_archive.h"

union STREAM;

typedef
  struct STREAM_CLASS {
    int (*open)(union STREAM *stream, const char *path, int mode, void *data);
    int (*close)(union STREAM *stream);
    int (*read)(union STREAM *stream, char *buffer, int len);
    int (*getchar)(union STREAM *stream, char *buffer);
    int (*write)(union STREAM *stream, char *buffer, int len);
    int (*seek)(union STREAM *stream, int64_t pos, int whence);
    int (*tell)(union STREAM *stream, int64_t *pos);
    int (*flush)(union STREAM *stream);
    int (*eof)(union STREAM *stream);
    int (*lof)(union STREAM *stream, int64_t *len);
    int (*handle)(union STREAM *stream);
    }
  STREAM_CLASS;

typedef
  struct {
    STREAM_CLASS *type;
    char *buffer;
    short buffer_pos;
    short buffer_len;
    short mode;
    unsigned swap : 1;
    unsigned eol : 2;
    unsigned eof : 1;
    unsigned no_fionread : 1;
    unsigned no_lseek : 1;
    unsigned available_now : 1;
		unsigned standard : 1;
    #if DEBUG_STREAM
    unsigned _reserved : 1;
    unsigned tag : 7;
    #else
    unsigned _reserved : 8;
    #endif
    }
  PACKED
  STREAM_COMMON;


typedef
  struct {
    STREAM_COMMON common;
    int _reserved[6];
    }
  PACKED
  STREAM_RESERVED;

typedef
  struct {
    STREAM_COMMON common;
    int64_t size;
    int fd;
    }
  PACKED
  STREAM_DIRECT;

typedef
  struct {
    STREAM_COMMON common;
    FILE *file;
    }
  PACKED
  STREAM_BUFFER;

typedef
  struct {
    STREAM_COMMON common;
    void *addr;
    int64_t pos;
    }
  PACKED
  STREAM_MEMORY;

typedef
  struct {
    STREAM_COMMON common;
    ARCHIVE *arch;
    int size;
    int start;
    int pos;
    }
  PACKED
  STREAM_ARCH;

typedef
  struct {
    STREAM_COMMON common;
    int fdr;
    int fdw;
    unsigned read_something : 1;
    unsigned _reserved : 31;
    }
  PACKED
  STREAM_PROCESS;


typedef
  union STREAM {
    STREAM_CLASS *type;
    STREAM_COMMON common;
    STREAM_RESERVED _reserved;
    STREAM_DIRECT direct;
    STREAM_BUFFER buffer;
    STREAM_DIRECT pipe;
    STREAM_MEMORY memory;
    STREAM_ARCH arch;
    STREAM_PROCESS process;
    }
  STREAM;

enum {
  ST_READ = 1,
  ST_WRITE = 2,
  ST_READ_WRITE = 3,
  ST_MODE = 3,
  ST_APPEND = 4,
  ST_CREATE = 8,
  ST_ACCESS = 15,
  ST_DIRECT = 16,
  ST_LINE = 32,
  ST_WATCH = 64,
  ST_PIPE = 128
  };

enum {
	ST_EOL_UNIX = 0,
	ST_EOL_WINDOWS = 1,
	ST_EOL_MAC = 2
	};

EXTERN int STREAM_eff_read;

#ifndef __STREAM_IMPL_C

EXTERN STREAM_CLASS STREAM_direct;
EXTERN STREAM_CLASS STREAM_buffer;
EXTERN STREAM_CLASS STREAM_pipe;
EXTERN STREAM_CLASS STREAM_memory;
EXTERN STREAM_CLASS STREAM_arch;
EXTERN STREAM_CLASS STREAM_process;
/*EXTERN STREAM_CLASS STREAM_null;*/

#else

#define DECLARE_STREAM(stream) \
STREAM_CLASS stream = \
{ \
  (void *)stream_open, \
  (void *)stream_close, \
  (void *)stream_read, \
  (void *)stream_getchar, \
  (void *)stream_write, \
  (void *)stream_seek, \
  (void *)stream_tell, \
  (void *)stream_flush, \
  (void *)stream_eof, \
  (void *)stream_lof, \
  (void *)stream_handle \
}

#endif

#define STREAM_BUFFER_SIZE 256

bool STREAM_in_archive(const char *path);
//int STREAM_get_readable(int fd, long *len);

void STREAM_open(STREAM *stream, const char *path, int mode);

void STREAM_release(STREAM *stream);
void STREAM_close(STREAM *stream);
void STREAM_write(STREAM *stream, void *addr, int len);
void STREAM_line_input(STREAM *stream, char **addr);
void STREAM_input(STREAM *stream, char **addr);
int64_t STREAM_tell(STREAM *stream);
void STREAM_seek(STREAM *stream, int64_t pos, int whence);
void STREAM_read(STREAM *stream, void *addr, int len);
char STREAM_getchar(STREAM *stream);
void STREAM_read_type(STREAM *stream, TYPE type, VALUE *value, int len);
void STREAM_write(STREAM *stream, void *addr, int len);
void STREAM_write_type(STREAM *stream, TYPE type, VALUE *value, int len);
void STREAM_write_eol(STREAM *stream);
void STREAM_flush(STREAM *stream);
int STREAM_handle(STREAM *stream);
void STREAM_lof(STREAM *stream, int64_t *len);
bool STREAM_eof(STREAM *stream);

void STREAM_load(const char *path, char **buffer, int *len);

bool STREAM_map(const char *path, char **paddr, int *plen);
void STREAM_unmap(char *addr, int len);

int STREAM_read_direct(int fd, char *buffer, int len);
int STREAM_write_direct(int fd, char *buffer, int len);
//int STREAM_read_buffered(FILE *file, char *buffer, long len);
//int STREAM_write_buffered(FILE *file, char *buffer, long len);

void STREAM_lock(STREAM *stream);

#define STREAM_is_closed(_stream) ((_stream)->type == NULL)

void STREAM_blocking(STREAM *stream, bool block);
bool STREAM_is_blocking(STREAM *stream);

#if DEBUG_STREAM
void STREAM_exit(void);
#else
#define STREAM_exit()
#endif

#endif
