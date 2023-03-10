/***************************************************************************

  gkey.h

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

#ifndef __GKEY_H
#define __GKEY_H

class gKey
{
public:
	static bool isValid() { return _valid; }
	static const char *text();
	static int code();
	static int state();

	static bool alt();
	static bool control();
	static bool meta();
	static bool normal();
	static bool shift();

	static int fromString(const char *str);

//"Private"
	static void disable();
	static bool enable(gControl *control, GdkEventKey *e);
	static bool canceled() { return _canceled; }
	static void init();
	static void exit();
	
	static void setActiveControl(gControl *control);

	static bool raiseEvent(int type, gControl *control, const char *text);

	static bool mustIgnoreEvent(GdkEventKey *e);

	static bool gotCommit();

	static bool _canceled;
	static GdkEventKey _event;
	static int _last_key_press;
	static int _last_key_release;

private:
	static int _valid;
	
	static void initContext();
};

void gcb_im_commit(GtkIMContext *context, const char *str, gControl *control);
gboolean gcb_key_event(GtkWidget *widget, GdkEvent *event, gControl *data);

#endif
