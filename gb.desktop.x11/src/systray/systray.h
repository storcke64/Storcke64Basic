/***************************************************************************

  systray.h

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

#ifndef __SYSTRAY_H
#define __SYSTRAY_H

#include "config.h"
#include "../x11.h"
#include "../c_x11systray.h"
#include "../main.h"

typedef
	struct TrayIcon
	CX11SYSTRAYICON;

//#define DEBUG

void SYSTRAY_init(Display *display, Window window, uint bg);
void SYSTRAY_exit();
int SYSTRAY_event_filter(XEvent *ev);
int SYSTRAY_get_count();
CX11SYSTRAYICON *SYSTRAY_get(int i);
void SYSTRAY_refresh(void);
void SYSTRAY_move(int x, int y, int w, int h);
void SYSTRAY_resize(int w, int h);

#endif
