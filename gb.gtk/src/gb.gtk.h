/***************************************************************************

  gb.gtk.h

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

#ifndef GB_GTK_H
#define GB_GTK_H

#include "gambas.h"
#include <cairo.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>

#ifndef GDK_WINDOWING_X11
#define NO_X_WINDOW 1
#endif

#define GTK_INTERFACE_VERSION 1

#ifdef GTK3
#define GTK_NAME "gb.gtk3"
#else
#define GTK_NAME "gb.gtk"
#endif

typedef
	void *GTK_PICTURE;

#define CCF_NONE                0
#define CCF_HAS_INPUT_METHOD    1

typedef
	struct 
	{
		intptr_t version;
		void (*CreateControl)(void *control, void *parent, GtkWidget *widget, uint flags);
		GtkWidget *(*CreateGLArea)(void *control, void *parent, void (*init)(GtkWidget *));
		GTK_PICTURE *(*CreatePicture)(cairo_surface_t *surf, int w, int h);
		int (*GetDesktopScale)(void);
		void *_null;
	}  
	GTK_INTERFACE;

typedef  
	struct {
	  GB_BASE ob;
	  void *widget;
		GB_VARIANT_VALUE tag;
		void *font;
		void *cursor;
		char *popup;
		char *action;
	}  
	GTK_CONTROL;

#endif
