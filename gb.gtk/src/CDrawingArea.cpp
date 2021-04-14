/***************************************************************************

  CDrawingArea.cpp

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

#define __CDRAWINGAREA_CPP

#include <stdio.h>

#include "main.h"
#include "gambas.h"
#include "widgets.h"
#include "gapplication.h"
#include "CDraw.h"
#include "cpaint_impl.h"
#include "CDrawingArea.h"
#include "CWidget.h"
#include "CContainer.h"

DECLARE_EVENT(EVENT_Draw);
DECLARE_EVENT(EVENT_Font);
DECLARE_EVENT(EVENT_Change);


/***************************************************************************

	DrawingArea

***************************************************************************/

static bool cb_change_filter(gControl *control)
{
	return control->isDrawingArea();
}

static void cb_change(gControl *control)
{
	GB.Raise(control->hFree, EVENT_Change, 0);
}

void CDRAWINGAREA_send_change_event(void)
{
	gApplication::forEachControl(cb_change, cb_change_filter);
}

#ifdef GTK3

typedef
	struct {
		CDRAWINGAREA *control;
		cairo_t *save;
	}
	HANDLER_INFO;

static void cleanup_drawing(HANDLER_INFO *info)
{
	PAINT_end();
	info->control->context = info->save;
}

static void cb_expose(gDrawingArea *sender, cairo_t *cr)
{
	CWIDGET *_object = GetObject(sender);
	GB_RAISE_HANDLER handler;
	HANDLER_INFO info;
	int fw;

	if (GB.CanRaise(THIS, EVENT_Draw))
	{
		handler.callback = (void (*)(intptr_t))cleanup_drawing;
		handler.data = (intptr_t)&info;

		info.control = THIS;
		info.save = THIS->context;
		
		GB.RaiseBegin(&handler);

		THIS->context = cr;
		PAINT_begin(THIS);

		fw = sender->getFrameWidth();
		cairo_save(cr);
		//cairo_reset_clip(cr);
		PAINT_clip(fw, fw, sender->width() - fw * 2, sender->height() - fw * 2);
		
		GB.Raise(THIS, EVENT_Draw, 0);
		
		cairo_restore(cr);

		PAINT_end();
		THIS->context = info.save;

		GB.RaiseEnd(&handler);
	}
}
#else
static void cleanup_drawing(intptr_t _unused)
{
	PAINT_end();
}

static void cb_expose(gDrawingArea *sender, GdkRegion *region, int dx, int dy)
{
	CWIDGET *_object = GetObject(sender);
	GB_RAISE_HANDLER handler;
	cairo_t *cr;
	int fw;

	if (GB.CanRaise(THIS, EVENT_Draw))
	{
		handler.callback = cleanup_drawing;
		handler.data = (intptr_t)THIS;

		GB.RaiseBegin(&handler);

		PAINT_begin(THIS);
		cr = PAINT_get_current_context();
		fw = sender->getFrameWidth();
		cairo_save(cr);
		PAINT_clip(fw, fw, sender->width() - fw * 2, sender->height() - fw * 2);

		GB.Raise(THIS, EVENT_Draw, 0);

		cairo_restore(cr);
		PAINT_end();

		GB.RaiseEnd(&handler);
	}
}
#endif

static void cb_font_change(gDrawingArea *sender)
{
	CWIDGET *_object = GetObject(sender);
	GB.Raise(THIS, EVENT_Font, 0);
}


BEGIN_METHOD(CDRAWINGAREA_new, GB_OBJECT parent)

	InitControl(new gDrawingArea(CONTAINER(VARG(parent))), (CWIDGET*)THIS);
	WIDGET->onExpose = cb_expose;
	WIDGET->onFontChange = cb_font_change;

END_METHOD

BEGIN_PROPERTY(DrawingArea_Border)

	if (READ_PROPERTY) { GB.ReturnInteger(WIDGET->getBorder()); return; }
	WIDGET->setBorder(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(DrawingArea_Cached)

	if (READ_PROPERTY) { GB.ReturnBoolean(WIDGET->cached()); return; }
	WIDGET->setCached(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(DrawingArea_Focus)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WIDGET->canFocus());
	else
		WIDGET->setCanFocus(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_METHOD_VOID(DrawingArea_Clear)

	if (DRAW.Paint.IsPainted(THIS))
	{
		GB.Error("DrawingArea is being painted");
		return;
	}

	WIDGET->clear();

END_METHOD

BEGIN_PROPERTY(DrawingArea_Painted)

	static bool deprecated = false;
	
	if (!deprecated)
	{
		deprecated = true;
		GB.Deprecated(GTK_NAME, "DrawingArea.Painted", NULL);
	}
	
	if (READ_PROPERTY)
		GB.ReturnBoolean(true);

END_PROPERTY

BEGIN_PROPERTY(DrawingArea_NoBackground)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WIDGET->hasNoBackground());
	else
		WIDGET->setNoBackground(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_METHOD(DrawingArea_Refresh, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h)

	int x, y, w, h;

	if (MISSING(x) && MISSING(y) && MISSING(w) && MISSING(h))
		WIDGET->refresh();
	else
	{
		x = VARGOPT(x, 0);
		y = VARGOPT(y, 0);
		w = VARGOPT(w, WIDGET->width());
		h = VARGOPT(h, WIDGET->height());
		
		WIDGET->refresh(x,y,w,h);
	}

END_METHOD

BEGIN_PROPERTY(DrawingArea_Tablet)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WIDGET->useTablet());
	else
		WIDGET->setUseTablet(VPROP(GB_BOOLEAN));

END_PROPERTY

GB_DESC CDrawingAreaDesc[] =
{
	GB_DECLARE("DrawingArea", sizeof(CDRAWINGAREA)), GB_INHERITS("Container"),

	GB_METHOD("_new", 0, CDRAWINGAREA_new, "(Parent)Container;"),

	ARRANGEMENT_PROPERTIES,

	GB_PROPERTY("Cached", "b", DrawingArea_Cached),
	GB_PROPERTY("Border", "i", DrawingArea_Border),
	GB_PROPERTY("Focus","b",DrawingArea_Focus),
	GB_PROPERTY("Painted", "b", DrawingArea_Painted),
	GB_PROPERTY("NoBackground", "b", DrawingArea_NoBackground),
	
	GB_PROPERTY("Tablet", "b", DrawingArea_Tablet),

	GB_METHOD("Clear", NULL, DrawingArea_Clear, NULL),
	GB_METHOD("Refresh", NULL, DrawingArea_Refresh, "[(X)i(Y)i(Width)i(Height)i]"),

	GB_EVENT("Draw", NULL, NULL, &EVENT_Draw),
	GB_EVENT("Font", NULL, NULL, &EVENT_Font),
	GB_EVENT("Change", NULL, NULL, &EVENT_Change),

	GB_INTERFACE("Paint", &PAINT_Interface),
	
	DRAWINGAREA_DESCRIPTION,

	GB_END_DECLARE
};


