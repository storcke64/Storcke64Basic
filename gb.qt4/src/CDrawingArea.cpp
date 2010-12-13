/***************************************************************************

  CDrawingArea.cpp

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

#define __CDRAWINGAREA_CPP

#include <QApplication>
#include <QPaintEvent>
#include <QPixmap>
#include <QPainter>

#include "CDraw.h"
#include "cpaint_impl.h"
#include "CDrawingArea.h"

#ifndef NO_X_WINDOW
#include <QX11Info>
#include <X11/Xlib.h>
#endif

DECLARE_EVENT(EVENT_draw);


/***************************************************************************

	class MyDrawingArea

***************************************************************************/

MyDrawingArea::MyDrawingArea(QWidget *parent) : MyContainer(parent)
{
	drawn = 0;
	cache = 0;
	_background = 0;
	_frozen = false;
	_event_mask = 0;
	_use_paint = false;
	_set_background = false;
	_cached = false;
	
	setMerge(false);
	setCached(false);
	setAllowFocus(false);
	
	setAttribute(Qt::WA_KeyCompression, false);
	setAttribute(Qt::WA_NativeWindow, true);
	setAttribute(Qt::WA_DontCreateNativeAncestors, true);
	
	//setAttribute(Qt::WA_NoSystemBackground, true);
}


MyDrawingArea::~MyDrawingArea()
{
	if (_background)
	{
		delete _background;
		_background = 0;
	}
}

void MyDrawingArea::setAllowFocus(bool f)
{
	if (f)
	{
		void *_object = CWidget::getReal(this);
		setFocusPolicy(GB.CanRaise(THIS, EVENT_MouseWheel) ? Qt::WheelFocus : Qt::StrongFocus);
		setAttribute(Qt::WA_InputMethodEnabled, true);
	}
	else
	{
		setFocusPolicy(Qt::NoFocus);
	}
}


void MyDrawingArea::setMerge(bool m)
{
	if (m)
		qDebug("warning: DrawingArea.Merge property has been deprecated");
	_merge = m;
}

void MyDrawingArea::setFrozen(bool f)
{
	if (f == _frozen)
		return;

	#ifndef NO_X_WINDOW
	XWindowAttributes attr;

	if (f)
	{
		//setBackgroundMode(Qt::NoBackground);
		XGetWindowAttributes(QX11Info::display(), winId(), &attr);
		_event_mask = attr.your_event_mask;
		XSelectInput(QX11Info::display(), winId(), ExposureMask);
		//clearWFlags(Qt::WPaintClever);
		//qDebug("frozen");
	}
	else
	{
		//setBackgroundMode(Qt::PaletteBackground);
		XSelectInput(QX11Info::display(), winId(), _event_mask);
		setMerge(_merge);
		//qDebug("unfrozen");
	}
	#endif
	
	_frozen = f;
}

void MyDrawingArea::redraw(QRect &r, bool frame)
{
	QPainter *p;
	void *_object = CWidget::getReal(this);
	
	if (!_object)
		return;
			
	//qDebug("paint: %d %d %d %d", event->rect().x(), event->rect().y(), event->rect().width(), event->rect().height());

	if (_use_paint)
	{
		PAINT_begin(THIS);
		p = PAINT_get_current();
	}
	else
	{
		DRAW_begin(THIS);
		p = DRAW_get_current();
	}
		
	/*if (!isTransparent())
	{
		p->translate(-r.x(), -r.y());
	}*/
	
	p->save();
	
	if (!_use_paint)
	{
		//p->setBrushOrigin(-r.x(), -r.y());
		DRAW_clip(r.x(), r.y(), r.width(), r.height());
	}
	else
		PAINT_clip(r.x(), r.y(), r.width(), r.height());
	
	//p->setClipRegion(event->region().intersect(contentsRect()));
	//p->setBrushOrigin(-r.x(), -r.y());
	
	GB.Raise(THIS, EVENT_draw, 0);
		
	p->restore();
	
	if (frame)
	{
		p->setRenderHint(QPainter::Antialiasing, false);
		drawFrame(p);
	}
		
	if (_use_paint)
		PAINT_end();
	else
		DRAW_end();
}

void MyDrawingArea::paintEvent(QPaintEvent *event)
{
	if (_background)
	{
		#ifndef NO_X_WINDOW
		if (_set_background)
		{
			XSetWindowBackgroundPixmap(QX11Info::display(), winId(), _background->handle());
			_set_background = false;
		}
		#endif
		QPainter paint( this );
		drawFrame(&paint);
		//MyContainer::paintEvent(event);
	}
	else
	{
		//QPainter paint( this );
		QRect r;

		r = event->rect().intersect(contentsRect());
		if (r.isValid())
		{
			/*if (!isTransparent())
			{
				cache = new QPixmap(r.width(), r.height());
				cache->fill(this, r.x(), r.y());
			}*/
			
			redraw(r, true);

			/*if (!isTransparent())
			{
				paint.drawPixmap(r.x(), r.y(), *cache);
				delete cache;
				cache = 0;
			}*/
		}
	}
}

void MyDrawingArea::setBackground()
{
	if (_background)
	{
		_background->detach();

		#ifdef NO_X_WINDOW
		setErasePixmap(*_background);
		#else
		//if (isVisible())
		//	XSetWindowBackgroundPixmap(QX11Info::display(), winId(), _background->handle());
		//else
		_set_background = true;
		refreshBackground();
		#endif
	}
}

void MyDrawingArea::refreshBackground()
{
	if (_background)
	{
		#ifdef NO_X_WINDOW
		update();
		#else
		int fw = frameWidth();
		if (fw == 0)
			XClearWindow(QX11Info::display(), winId());
		else
		{
			XClearArea(QX11Info::display(), winId(), fw, fw, 0, 0, False);
			repaint();
		}
		#endif
	}
}


void MyDrawingArea::clearBackground()
{
	if (_background)
	{
		QPainter p(_background);
		p.fillRect(0, 0, _background->width(), _background->height(), palette().color(backgroundRole()));
		p.end();
		setBackground();
	}
}

void MyDrawingArea::resizeEvent(QResizeEvent *e)
{
	MyContainer::resizeEvent(e);
	updateBackground();
}

void MyDrawingArea::updateBackground()
{
	int wb, hb, w, h;

	if (drawn)
	{
		GB.Error("DrawingArea is being drawn");
		return;
	}

	if (_background)
	{
		w = QMAX(width(), 1);
		h = QMAX(height(), 1);

		if (w != _background->width() && h != _background->height())
		{		
			QPixmap *p = new QPixmap(w, h);
			p->fill(palette().color(backgroundRole()));

			wb = QMIN(w, _background->width());
			hb = QMIN(h, _background->height());

			QPainter pt(p);
			pt.drawPixmap(0, 0, *_background, 0, 0, wb, hb);
			pt.end();

			delete _background;
			_background = p;
			
			setBackground();
		}
	}
}

void MyDrawingArea::setStaticContents(bool on)
{
}


void MyDrawingArea::updateCache()
{
	if (_background)
		delete _background;

	if (_cached) // && !_transparent)
	{
		_background = new QPixmap(width(), height());
		clearBackground();

		setAttribute(Qt::WA_PaintOnScreen, true);
		setAttribute(Qt::WA_OpaquePaintEvent, true);
		setAttribute(Qt::WA_StaticContents, true);
	}
	else //if (_background)
	{
		_background = 0;
		setAttribute(Qt::WA_PaintOnScreen, false);
		setAttribute(Qt::WA_OpaquePaintEvent, false);
		setAttribute(Qt::WA_StaticContents, false);
		#ifdef NO_X_WINDOW
		setBackgroundMode(Qt::NoBackground);
		#else
		XSetWindowBackgroundPixmap(QX11Info::display(), winId(), None);
		XClearArea(QX11Info::display(), winId(), 0, 0, 0, 0, True);
		#endif
	}
}

void MyDrawingArea::setCached(bool c)
{
	_cached = c;
	updateCache();
}

void MyDrawingArea::setPalette(const QPalette &pal)
{
	MyContainer::setPalette(pal);
	repaint();
}

/*void MyDrawingArea::setTransparent(bool on)
{
	_transparent = on;
	updateCache();
}*/

void MyDrawingArea::hideEvent(QHideEvent *e)
{
	if (_background)
		_set_background = true;
	MyContainer::hideEvent(e);
}

/***************************************************************************

	DrawingArea

***************************************************************************/

BEGIN_METHOD(CDRAWINGAREA_new, GB_OBJECT parent)

	MyDrawingArea *wid = new MyDrawingArea(QCONTAINER(VARG(parent)));

	//THIS->widget.background = QColorGroup::Base;
	THIS->container = wid;
	THIS->widget.flag.fillBackground = false;

	CWIDGET_new(wid, (void *)_object);

END_METHOD


BEGIN_PROPERTY(CDRAWINGAREA_cached)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WIDGET->isCached());
	else
		WIDGET->setCached(VPROP(GB_BOOLEAN));

END_PROPERTY


DECLARE_METHOD(Control_Background);


BEGIN_METHOD_VOID(CDRAWINGAREA_clear)

	WIDGET->clearBackground();

END_METHOD


BEGIN_PROPERTY(CDRAWINGAREA_background)

	Control_Background(_object, _param);

	if (!READ_PROPERTY)
		WIDGET->clearBackground();

END_PROPERTY

BEGIN_PROPERTY(CDRAWINGAREA_border)

	CCONTAINER_border(_object, _param);

	if (!READ_PROPERTY)
	{
		WIDGET->clearBackground();
	}

END_PROPERTY

BEGIN_PROPERTY(CDRAWINGAREA_enabled)

	Control_Enabled(_object, _param);

	if (!READ_PROPERTY)
		WIDGET->setFrozen(!VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CDRAWINGAREA_merge)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WIDGET->isMerge());
	else
		WIDGET->setMerge(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CDRAWINGAREA_focus)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WIDGET->isAllowFocus());
	else
		WIDGET->setAllowFocus(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CDRAWINGAREA_painted)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WIDGET->isPaint());
	else
		WIDGET->setPaint(VPROP(GB_BOOLEAN));

END_PROPERTY

#if 0
BEGIN_PROPERTY(CDRAWINGAREA_transparent)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WIDGET->isTransparent());
	else
	{
		WIDGET->setTransparent(VPROP(GB_BOOLEAN));
		//THIS->widget.flag.fillBackground = !WIDGET->isTransparent();
		//CWIDGET_reset_color((CWIDGET *)THIS);
	}

END_PROPERTY
#endif

BEGIN_METHOD(DrawingArea_Refresh, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h)

	if (WIDGET->isCached())
	{
		QRect r;
		
		if (!MISSING(x) && !MISSING(y))
			r.setRect(VARG(x), VARG(y), VARGOPT(w, WIDGET->width()), VARGOPT(h, WIDGET->height()));
		else
			r.setRect(0, 0, WIDGET->width(), WIDGET->height());
		
		WIDGET->redraw(r, false);
	}
	
	Control_Refresh(_object, _param);

END_METHOD

GB_DESC CDrawingAreaDesc[] =
{
	GB_DECLARE("DrawingArea", sizeof(CDRAWINGAREA)), GB_INHERITS("Container"),

	GB_METHOD("_new", NULL, CDRAWINGAREA_new, "(Parent)Container;"),

	GB_PROPERTY("Cached", "b", CDRAWINGAREA_cached),
	GB_PROPERTY("Merge", "b", CDRAWINGAREA_merge),
	
  GB_PROPERTY("Arrangement", "i", CCONTAINER_arrangement),
  GB_PROPERTY("AutoResize", "b", CCONTAINER_auto_resize),
  GB_PROPERTY("Spacing", "b", CCONTAINER_spacing),
  GB_PROPERTY("Margin", "b", CCONTAINER_margin),
  GB_PROPERTY("Padding", "i", CCONTAINER_padding),
  GB_PROPERTY("Indent", "b", CCONTAINER_indent),

	GB_PROPERTY("Border", "i", CDRAWINGAREA_border),
	GB_PROPERTY("Background", "i", CDRAWINGAREA_background),
	
	GB_PROPERTY("Focus", "b", CDRAWINGAREA_focus),
	GB_PROPERTY("Enabled", "b", CDRAWINGAREA_enabled),
	GB_PROPERTY("Painted", "b", CDRAWINGAREA_painted),
	//GB_PROPERTY("Transparent", "b", CDRAWINGAREA_transparent),

	GB_METHOD("Clear", NULL, CDRAWINGAREA_clear, NULL),
	GB_METHOD("Refresh", NULL, DrawingArea_Refresh, "[(X)i(Y)i(Width)i(Height)i]"),

	GB_EVENT("Draw", NULL, NULL, &EVENT_draw),

	GB_INTERFACE("Draw", &DRAW_Interface),
	GB_INTERFACE("Paint", &PAINT_Interface),

	DRAWINGAREA_DESCRIPTION,

	GB_END_DECLARE
};




