/***************************************************************************

  gslider.h

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

#ifndef __GSLIDER_H
#define __GSLIDER_H

class gSlider : public gControl
{
public:
	gSlider(gContainer *parent, bool scrollbar = false);

	bool isScrollBar() const { return _is_scrollbar; }
	
//"Properties"
	
	int max() const { return _max; }
	int min() const { return _min; }
	bool tracking() const { return _tracking; }
	int value() const { return _value; }
	bool mark() const { return _mark; }
	int step() const { return _step; }
	int pageStep() const { return _page_step; }

	//void setForeground(int vl);
	//void setBackground(int vl);
	void setMax(int vl);
	void setMin(int vl);
	void setTracking(bool vl);
	void setValue(int vl);
	void setMark(bool vl);
	void setStep(int vl);
	void setPageStep(int vl);
	
	int getDefaultSize();
	bool isVertical() const;
	
	virtual bool resize(int w, int h);

//"Signals"
	void (*onChange)(gSlider *sender);

//"Private"
	void updateMark();
	void init();
	void update();
	void checkInverted();
	
	unsigned _mark : 1;
	unsigned _tracking : 1;
	unsigned _is_scrollbar : 1;
	
	int _step;
	int _page_step;
	int _value;
	int _min, _max;
};

#endif
