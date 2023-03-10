/***************************************************************************

  CDrawingArea.h

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

#ifndef __CDRAWINGAREA_H
#define __CDRAWINGAREA_H

#include <QPaintEvent>
#include <QPixmap>
#include <QEvent>
//#include <QFrame>

#include "gambas.h"

#include "CWidget.h"
#include "CContainer.h"

#ifndef __CDRAWINGAREA_CPP
extern GB_DESC CDrawingAreaDesc[];
#else

#define THIS    ((CDRAWINGAREA *)_object)
#define WIDGET  ((MyDrawingArea *)((CWIDGET *)_object)->widget)

#endif

typedef
	struct {
		CWIDGET widget;
		QWidget *container;
		CARRANGEMENT arrangement;
		}
	CDRAWINGAREA;

class MyDrawingArea : public MyContainer
{
	Q_OBJECT

	friend class MyScrollArea;

public:

	explicit MyDrawingArea(QWidget *parent);
	~MyDrawingArea();

	int drawn;
	QPixmap *cache;

	virtual void setVisible(bool visible);

	//void setTransparent(bool);
	//bool isTransparent(void) { return transparent; }

	void updateCache();
	void setCached(bool);
	bool isCached() const { return _cached; }

	void clearBackground();
#ifndef QT5
	Qt::HANDLE background() const { return _background; }
#endif
	void refreshBackground();
	void updateBackground();

	void setFrozen(bool f);
	bool isFrozen() const { return _frozen; }

	void redraw(QRect &r, bool frame = false);

	bool hasNoBackground() const { return _no_background; }
	void setNoBackground(bool on);
	void updateNoBackground();

	void setDrawEvent(int event) { _draw_event = event; }
	bool inDrawEvent() const { return _in_draw_event; }
	static bool inAnyDrawEvent() { return _in_any_draw_event > 0; }

	void createBackground(int w, int h);
#ifndef QT5
	bool hasCacheBackground() const { return _cached && _background; }
#else
	bool hasCacheBackground() const { return _cached && !_background_pixmap.isNull(); }
#endif
	void deleteBackground();

	QPixmap *getBackgroundPixmap();

public slots:

	void setBackground();
	//bool isTransparent() { return _transparent; }
	//void setTransparent(bool on);

protected:

	virtual void setStaticContents(bool on);
	virtual void resizeEvent(QResizeEvent *);
	virtual void paintEvent(QPaintEvent *);
	virtual void hideEvent(QHideEvent *);
	//virtual void drawContents(QPainter *p);
	virtual void setPalette(const QPalette &);
	virtual void changeEvent(QEvent *);

private:

	QPixmap _background_pixmap;
#ifndef QT5
	Qt::HANDLE _background;
#endif
	int _background_w, _background_h;
	unsigned _frozen : 1;
	unsigned _merge : 1;
	unsigned _focus : 1;
	unsigned _set_background : 1;
	unsigned _cached : 1;
	unsigned _no_background : 1;
	unsigned _in_draw_event : 1;
	int _event_mask;
	int _draw_event;
	static int _in_any_draw_event;
};

void CDRAWINGAREA_send_change_event(void);

#endif
