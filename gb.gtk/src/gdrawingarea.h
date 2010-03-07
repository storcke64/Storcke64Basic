/***************************************************************************

  gdrawingarea.h

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
#ifndef __GDRAWINGAREA_H
#define __GDRAWINGAREA_H

class gDrawingArea : public gContainer
{
public:
	gDrawingArea(gContainer *parent);
	~gDrawingArea();

	int getBorder() const { return getFrameBorder(); }
	bool cached() const { return _cached; }
	bool canFocus() const;
	bool isTransparent() const { return _transparent; }

	void setBorder(int vl) { setFrameBorder(vl); }
	void setCached(bool vl);
	void setCanFocus(bool vl);
	void setTransparent(bool vl);

//"Methods"
	void clear();
	virtual void resize(int w, int h);
	virtual void setEnabled(bool vl);

//"Events"
	void (*onExpose)(gDrawingArea *sender,int x,int y,int w,int h);

//"Private"
	void updateCache();
	void resizeCache();
	void refreshCache();
	void updateEventMask();
	void setCache();
	GdkPixmap *buffer;
	uint _event_mask;
	uint _old_bg_id;
	unsigned _cached : 1;
	unsigned _resize_cache : 1;
	unsigned _transparent;
};

#endif
