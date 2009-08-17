/***************************************************************************

  gdesktop.h

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
#ifndef __GDESKTOP_H
#define __GDESKTOP_H

class gPicture;
class gMainWindow;
class gControl;
class gFont;

class gDesktop
{
public:

	static void init();
	static void exit();

	static gColor buttonfgColor();
	static gColor buttonbgColor();
	static gColor fgColor();
	static gColor bgColor();
	static gColor textfgColor();
	static gColor textbgColor();
	static gColor selfgColor();
	static gColor selbgColor();

	static gFont* font();
	static void setFont(gFont *vl);
	static int height();
	static int width();
	static int resolution();
	static int scale();
	static gPicture* grab();
	static gMainWindow* activeWindow();

	static bool rightToLeft();
	
	static gControl* activeControl() { return _active_control; }
	static void setActiveControl(gControl *control);
	
private:

	static gControl *_active_control;
	static int _desktop_scale;
	static gFont *_desktop_font;
};

#endif
