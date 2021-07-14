/***************************************************************************

  watcher.cpp

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>

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

#include "main.h"
#include "gambas.h"
#include "watcher.h"

//#define DEBUG_ME 1

static WATCH **watch = NULL;

static gboolean watch_adaptor(GIOChannel *source, GIOCondition condition, gpointer param)
{
	WATCH *data = (WATCH *)param;

#if DEBUG_ME
	fprintf(stderr, "watch_adaptor: %p: condition = %d, data = %p\n", source, (int)condition, (void *)param);
#endif

	if (!data) return true;
	
	if (condition & G_IO_IN)
		(*data->callback_read)(data->fd, GB_WATCH_READ, data->param_read);
	else if (condition & G_IO_OUT)
		(*data->callback_write)(data->fd, GB_WATCH_WRITE, data->param_write);

	return true;
}

void CWatcher::init()
{
	GB.NewArray(POINTER(&watch), sizeof(WATCH *), 0);
}

void CWatcher::exit()
{
	Clear();
	GB.FreeArray(POINTER(&watch));
}

void CWatcher::Clear()
{
	while (count()) 
	{
		CWatcher::Add(watch[0]->fd, GB_WATCH_NONE, NULL, 0);
	}
}

void CWatcher::Remove(int fd)
{
	CWatcher::Add(fd, GB_WATCH_NONE, NULL, 0);
}

void CWatcher::Add(int fd, int type, void *callback, intptr_t param)
{
	WATCH *data = NULL;
	WATCH **pwatch;
	int i;

	for (i = 0; i < count(); i++)
	{
		if (watch[i]->fd == fd)
		{
			data = watch[i];
			break;
		}
	}
	
	if (!data)
	{
		if (type == GB_WATCH_NONE || !callback)
			return;
		
		pwatch = (WATCH **)GB.Add(&watch);

		GB.Alloc(POINTER(pwatch), sizeof(WATCH));
		data = *pwatch;
		data->fd = fd;
		data->channel_read = data->channel_write = 0;
		data->callback_read = data->callback_write = 0;
	}

	if (data->callback_read && (type == GB_WATCH_NONE || type == GB_WATCH_READ))
	{
#if DEBUG_ME
		fprintf(stderr, "remove watch on fd %d for read (%p)\n", data->fd, data->channel_read);
#endif
		g_source_remove(data->id_read);
		g_io_channel_unref(data->channel_read);
		data->callback_read = 0;
		data->channel_read = 0;
	}
	
	if (data->callback_write && (type == GB_WATCH_NONE || type == GB_WATCH_WRITE))
	{
#if DEBUG_ME
		fprintf(stderr, "remove watch on fd %d for read (%p)\n", data->fd, data->channel_write);
#endif
		g_source_remove(data->id_write);
		g_io_channel_unref(data->channel_write);
		data->callback_write = 0;
		data->channel_write = 0;
	}
	
	if (callback)
	{
		if (type == GB_WATCH_READ)
		{
			data->callback_read = (WATCH_CALLBACK)callback;
			data->param_read = param;
			data->channel_read = g_io_channel_unix_new(fd);
			g_io_channel_set_encoding(data->channel_read, NULL, NULL);
			g_io_channel_set_buffered(data->channel_read, FALSE);
			data->id_read = g_io_add_watch_full(data->channel_read, G_PRIORITY_DEFAULT, G_IO_IN, watch_adaptor, (void*)data, NULL);
#if DEBUG_ME
			fprintf(stderr, "add watch on fd %d for read (%p)\n", fd, data->channel_read);
#endif
		}
		else if (type == GB_WATCH_WRITE)
		{
			data->callback_write = (WATCH_CALLBACK)callback;
			data->param_write = param;
			data->channel_write = g_io_channel_unix_new(fd);
			g_io_channel_set_encoding(data->channel_write, NULL, NULL);
			g_io_channel_set_buffered(data->channel_write, FALSE);
			data->id_write = g_io_add_watch_full(data->channel_write, G_PRIORITY_DEFAULT, G_IO_OUT, watch_adaptor, (void*)data, NULL);
#if DEBUG_ME
			fprintf(stderr, "add watch on fd %d for write (%p)\n", fd, data->channel_write);
#endif
		}
	}

	if (!data->callback_read && !data->callback_write)
	{
		GB.Free(POINTER(&data));
		GB.Remove(&watch, i, 1);
		MAIN_check_quit();
	}
}

int CWatcher::count()
{
	return GB.Count(watch);
}
