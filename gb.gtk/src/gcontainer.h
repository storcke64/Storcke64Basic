/***************************************************************************

  gcontainer.h

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

#ifndef __GCONTAINER_H
#define __GCONTAINER_H

#include "gcontrol.h"

#ifdef GTK3
void CUSERCONTROL_cb_draw(gContainer *sender, cairo_t *cr);
#else
void CUSERCONTROL_cb_draw(gContainer *sender, GdkRegion *region, int dx, int dy);
#endif
void CUSERCONTROL_cb_font(gContainer *sender);

struct gContainerArrangement
{
	unsigned mode : 4;
	unsigned user : 1;
	unsigned locked : 1;
	unsigned margin : 1;
	unsigned spacing : 1;
	unsigned padding : 8;
	unsigned indent : 1;
	unsigned centered : 1;
	unsigned dirty : 1;
	unsigned autoresize : 1;
	unsigned invert : 1;
	unsigned paint : 1;
	unsigned _reserved: 10;
}; 

class gContainer : public gControl
{
public:
	gContainer();
	gContainer(gContainer *parent);
	virtual ~gContainer();

	int arrange() const { return arrangement.mode; }
	bool autoResize() const { return arrangement.autoresize; }
	bool isUser() const { return arrangement.user; }
	int padding() const { return arrangement.padding; }
	bool spacing() const { return arrangement.spacing; }
	bool margin() const { return arrangement.margin; }
	bool indent() const { return arrangement.indent; }
	bool invert() const { return arrangement.invert; }
	bool centered() const { return arrangement.centered; }
	bool isPaint() const { return arrangement.paint; }
	bool isArranging() const { return arrangement.locked; }
	
	virtual int clientWidth();
	virtual int clientHeight();
	virtual int clientX();
	virtual int clientY();
	virtual int containerX();
	virtual int containerY();
	virtual int containerWidth();
	virtual int containerHeight();

	void setArrange(int vl);
	void setUser();
	void setPaint();
	void setAutoResize(bool vl);
	void setPadding(int vl);
	void setSpacing(bool vl);
	void setMargin(bool vl);
	void setIndent(bool vl);
	void setInvert(bool vl);
	void setCentered(bool vl);

	void setUserContainer() { _user_container = true; }
	bool isUserContainer() const { return _user_container; }
	
	virtual int childCount() const;
	virtual gControl *child(int index) const;
	gControl *firstChild() const { return child(0); };
	gControl *lastChild() const { return child(childCount() - 1); }
	
	int childIndex(gControl *ch) const;
	
	int childRec(int index) const;
	int childCountRec(int index) const;
	
	void clear();
	
	virtual gControl *find(int x, int y);
	
	gContainerArrangement *getArrangement() { return &arrangement; }
	gContainerArrangement fullArrangement() { return arrangement; }
	void setFullArrangement(gContainerArrangement *arr);
	
	virtual void performArrange();
	void decide(gControl *control, bool *width, bool *height);
	void getMaxSize(int xc, int yc, int wc, int hc, int *w, int *h);

#ifndef GTK3
	virtual void setBackground(gColor color = COLOR_DEFAULT);
#endif
	virtual void setForeground(gColor color = COLOR_DEFAULT);
	virtual void updateFont();
	
	bool hasBackground() const;
	bool hasForeground() const;

	virtual bool resize(int w, int h, bool no_decide = false);

	virtual void setVisible(bool vl);

	gContainer *proxyContainer() { return _proxyContainer ? _proxyContainer : this; }
	void setProxyContainer(gContainer *proxy);
	gContainer *proxyContainerFor() { return _proxyContainerFor; }
	void setProxyContainerFor(gContainer *proxy) { if (proxy != this) _proxyContainerFor = proxy; else _proxyContainerFor = NULL; }
	
	virtual void setDesign(bool ignore = false);
	
	void disableArrangement();
	void enableArrangement();
	bool isArrangementEnabled() const { return _no_arrangement == 0; }
	
//"Signals"
	void (*onArrange)(gContainer *sender);
	void (*onBeforeArrange)(gContainer *sender);
	//void (*onInsert)(gContainer *sender, gControl *child);

//"Private"
	GtkWidget *radiogroup;
	GPtrArray *_children;
	int _client_x, _client_y, _client_w, _client_h;
	
	virtual void insert(gControl *child, bool realize = false);
	virtual void remove(gControl *child);
	virtual void moveChild(gControl *child, int x, int y);
	virtual void reparent(gContainer *newpr, int x, int y);
	void hideHiddenChildren();
	virtual GtkWidget *getContainer();

	void setShown(bool v) { _shown = v; }
	bool isShown() const { return _shown; }
	
	virtual void connectBorder();
	
private:

	void initialize();
	void updateDesignChildren();

	gContainerArrangement arrangement;
  gContainer *_proxyContainer;
  gContainer *_proxyContainerFor;
	unsigned _did_arrangement : 1;
	unsigned _user_container : 1;
	unsigned _shown : 1;
	unsigned char _no_arrangement;
};

#endif
