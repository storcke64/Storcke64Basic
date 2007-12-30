/***************************************************************************

  CDraw.cpp

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>
  
  GTK+ component

  Realizado para la Junta de Extremadura. 
  Consejería de Educación Ciencia y Tecnología. 
  Proyecto gnuLinEx
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#define __CDRAW_CPP

#include "CWindow.h"
#include "CDrawingArea.h"
#include "CPicture.h"
#include "CFont.h"
#include "CDraw.h"

static void *CLASS_Window;
static void *CLASS_Picture;
static void *CLASS_Drawing;
static void *CLASS_DrawingArea;
static void *CLASS_Printer;

#define MAX_DRAWS 8
gDraw* drs[MAX_DRAWS];
int ndrs=-1;

gDraw* dr=NULL;


gDraw* DRAW_begin_widget(gControl *widget)
{
	ndrs++;
	dr=drs[ndrs];
	dr->connect(widget);
	return dr;
}

void DRAW_end_widget()
{
	if (dr)
	{
		if (dr->tag)
			GB.Unref((void **)POINTER(&dr->tag));
		dr->disconnect();
		ndrs--;
		
		dr= (ndrs>=0)? drs[ndrs] : NULL;
	}
}

static bool check_painter(void)
{
  if (!dr)
  {
    GB.Error("No current device");
    return true;
  }
  else
    return false;
}

#define CHECK_PAINTER() if (check_painter()) return



BEGIN_METHOD_VOID(CDRAW_init)

	long bucle;

	CLASS_Window = GB.FindClass("Window");
	CLASS_Picture = GB.FindClass("Picture");
	CLASS_Drawing = GB.FindClass("Drawing");
	CLASS_DrawingArea = GB.FindClass("DrawingArea");
	CLASS_Printer = GB.FindClass("Printer");

	for (bucle=0;bucle<MAX_DRAWS;bucle++) drs[bucle]=new gDraw(); 
	
END_METHOD


BEGIN_METHOD_VOID(CDRAW_exit)

	long bucle;

	for (bucle=0;bucle<MAX_DRAWS;bucle++) delete drs[bucle];

END_METHOD


BEGIN_METHOD(CDRAW_begin, GB_OBJECT device)

	void *Device=VARG(device);

  if (GB.CheckObject(Device))
    return;

	if (ndrs>(MAX_DRAWS-1)) 
	{ 
		GB.Error("Too many nested drawings"); 
		return; 
	}
	
	if (GB.Is(Device,CLASS_Window))
		DRAW_begin_widget( ((CWINDOW*)Device)->widget);
	else if (GB.Is(Device,CLASS_DrawingArea))
		DRAW_begin_widget( ((CDRAWINGAREA*)Device)->widget );
	else if (GB.Is(Device,CLASS_Picture))
	{
		ndrs++;
		dr=drs[ndrs];
		dr->tag = Device;
		dr->connect( ((CPICTURE*)Device)->picture);
	}
	else
	{
		GB.Error("Bad device");
		return;
	}

	dr->tag = Device;
	GB.Ref(Device);

END_METHOD


BEGIN_METHOD_VOID(CDRAW_end)

	DRAW_end_widget();
	
END_METHOD

BEGIN_PROPERTY(CDRAW_device)
	
  CHECK_PAINTER();

	GB.ReturnObject(dr->tag);

END_PROPERTY

BEGIN_PROPERTY(CDRAW_width)

  CHECK_PAINTER();

	GB.ReturnInteger(dr->width());

END_PROPERTY

BEGIN_PROPERTY(CDRAW_height)

  CHECK_PAINTER();

	GB.ReturnInteger(dr->height());

END_PROPERTY


BEGIN_PROPERTY(CDRAW_resolution)

  CHECK_PAINTER();

	GB.ReturnInteger(dr->resolution());

END_PROPERTY


BEGIN_PROPERTY(CDRAW_background)

	CHECK_PAINTER();
	
	if (READ_PROPERTY) { GB.ReturnInteger(dr->background()); return; }
	dr->setBackground(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CDRAW_foreground)

	CHECK_PAINTER();
	
	if (READ_PROPERTY) { GB.ReturnInteger(dr->foreground()); return; }
	dr->setForeground(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CDRAW_transparent)

	stub("CDRAW_transparent");

END_PROPERTY


BEGIN_PROPERTY(CDRAW_invert)

	CHECK_PAINTER();
	
	if (READ_PROPERTY) { GB.ReturnBoolean(dr->invert()); return; }
	dr->setInvert(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CDRAW_line_width)

	CHECK_PAINTER();
	
	if (READ_PROPERTY) { GB.ReturnInteger(dr->lineWidth()); return; }
	dr->setLineWidth(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CDRAW_line_style)

	CHECK_PAINTER();
	
	if (READ_PROPERTY) { GB.ReturnInteger(dr->lineStyle()); return; }
	dr->setLineStyle(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CDRAW_fill_color)

	CHECK_PAINTER();
		
	if (READ_PROPERTY) { GB.ReturnInteger(dr->fillColor()); return; }
	dr->setFillColor(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CDRAW_fill_style)

	CHECK_PAINTER();
		
	if (READ_PROPERTY) { GB.ReturnInteger(dr->fillStyle()); return; }
	dr->setFillStyle(VPROP(GB_INTEGER));


END_PROPERTY


BEGIN_PROPERTY(CDRAW_fill_x)

	CHECK_PAINTER();
		
	if (READ_PROPERTY) { GB.ReturnInteger(dr->fillX()); return; }
	dr->setFillX(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CDRAW_fill_y)

	CHECK_PAINTER();
		
	if (READ_PROPERTY) { GB.ReturnInteger(dr->fillY()); return; }
	dr->setFillY(VPROP(GB_INTEGER));

END_PROPERTY


static void set_font(gFont *font, void *object = 0)
{
  dr->setFont(font);
}

BEGIN_PROPERTY(CDRAW_font)

	CHECK_PAINTER();
		
	if (READ_PROPERTY)
    GB.ReturnObject(CFONT_create(dr->font(), set_font));
  else if (VPROP(GB_OBJECT))
    set_font(((CFONT*)VPROP(GB_OBJECT))->font);

END_PROPERTY


BEGIN_METHOD(CDRAW_rect, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h)

	CHECK_PAINTER();
	dr->rect(VARG(x),VARG(y),VARG(w),VARG(h));

END_METHOD


BEGIN_METHOD(CDRAW_ellipse, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_FLOAT start; GB_FLOAT len)

	long Start=0,Len=360;

	CHECK_PAINTER();
	
	if (!MISSING(start)) Start=VARG(start);
	if (!MISSING(len)) Len=VARG(len);
	
	dr->ellipse(VARG(x),VARG(y),VARG(w),VARG(h),Start,Len);


END_METHOD


BEGIN_METHOD(CDRAW_line, GB_INTEGER x1; GB_INTEGER y1; GB_INTEGER x2; GB_INTEGER y2)

	CHECK_PAINTER();
	dr->line(VARG(x1),VARG(y1),VARG(x2),VARG(y2));


END_METHOD


BEGIN_METHOD(CDRAW_point, GB_INTEGER x; GB_INTEGER y)

	CHECK_PAINTER();
	dr->point(VARG(x),VARG(y));

END_METHOD


BEGIN_METHOD(CDRAW_picture, GB_OBJECT pict; GB_INTEGER x; GB_INTEGER y; GB_INTEGER sx; GB_INTEGER sy; GB_INTEGER sw; GB_INTEGER sh)

	CPICTURE *Pict=(CPICTURE*)VARG(pict);
	long Sx=0,Sy=0,Sh=-1,Sw=-1;
	
	CHECK_PAINTER();
	if (!Pict) return;
	
	if (!MISSING(sx)) Sx=VARG(sx);
	if (!MISSING(sy)) Sy=VARG(sy);
	if (!MISSING(sw)) Sw=VARG(sw);
	if (!MISSING(sh)) Sh=VARG(sh);
	
	dr->picture(Pict->picture,VARG(x),VARG(y),Sx,Sy,Sw,Sh);

END_METHOD

BEGIN_METHOD(CDRAW_zoom, GB_OBJECT img; GB_INTEGER Zoom; GB_INTEGER x; GB_INTEGER y; GB_INTEGER sx; GB_INTEGER sy; GB_INTEGER sw; GB_INTEGER sh)

	stub("CDRAW_zoom");

END_METHOD

BEGIN_METHOD(CDRAW_image, GB_OBJECT img; GB_INTEGER x; GB_INTEGER y; GB_INTEGER sx; GB_INTEGER sy; GB_INTEGER sw; GB_INTEGER sh)

	CIMAGE *Img=(CIMAGE*)VARG(img);
	long Sx=0,Sy=0,Sh=-1,Sw=-1;
	
	CHECK_PAINTER();
	if (!Img) return;
	
	if (!MISSING(sx)) Sx=VARG(sx);
	if (!MISSING(sy)) Sy=VARG(sy);
	if (!MISSING(sw)) Sw=VARG(sw);
	if (!MISSING(sh)) Sh=VARG(sh);
	
	dr->picture(Img->picture,VARG(x),VARG(y),Sx,Sy,Sw,Sh);

END_METHOD


BEGIN_METHOD(CDRAW_drawing, GB_OBJECT drawing; GB_INTEGER x; GB_INTEGER y; GB_INTEGER sx; GB_INTEGER sy; GB_INTEGER sw; GB_INTEGER sh)

	stub("CDRAW_drawing");

END_METHOD


BEGIN_METHOD(CDRAW_tile, GB_OBJECT pict; GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h)

	stub("CDRAW_tile");

END_METHOD



BEGIN_METHOD(CDRAW_text, GB_STRING text; GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_INTEGER align)

	long x,y,w=-1,h=-1,align=-1;
	char *buf="";

	CHECK_PAINTER();	
	
	x=VARG(x);
	y=VARG(y);
	if (!MISSING(w)) w=VARG(w);
	if (!MISSING(h)) h=VARG(h);
	if (!MISSING(align)) align=VARG(align);
	dr->text(STRING(text),LENGTH(text),x,y,w,h,align);

END_METHOD


BEGIN_METHOD(CDRAW_text_width, GB_STRING text)

	CHECK_PAINTER();
	GB.ReturnInteger ( dr->textWidth( GB.ToZeroString(ARG(text)) ) );

END_METHOD


BEGIN_METHOD(CDRAW_text_height, GB_STRING text)

	CHECK_PAINTER();
	GB.ReturnInteger ( dr->textHeight( GB.ToZeroString(ARG(text)) ) );

END_METHOD



BEGIN_METHOD(CDRAW_polyline, GB_OBJECT points)

	long *np=NULL,bucle;
	GB_ARRAY Points=VARG(points);
	
	CHECK_PAINTER();
	if (!Points) return;
	
	GB.Alloc ((void**)&np,GB.Array.Count(Points)*sizeof(long));
	
	for (bucle=0;bucle<GB.Array.Count(Points);bucle++)
	{
		 np[bucle]=*((long*)GB.Array.Get(Points,bucle));
	}
	
	dr->polyline(np,GB.Array.Count(Points));
	GB.Free ((void**)&np);	

END_METHOD


BEGIN_METHOD(CDRAW_polygon, GB_OBJECT points)

	long *np=NULL,bucle;
	GB_ARRAY Points=VARG(points);
	
	CHECK_PAINTER();
	if (!Points) return;
	
	GB.Alloc ((void**)&np,GB.Array.Count(Points)*sizeof(long));
	
	for (bucle=0;bucle<GB.Array.Count(Points);bucle++)
	{
		 np[bucle]=*((long*)GB.Array.Get(Points,bucle));
	}
	
	dr->polygon(np,GB.Array.Count(Points));
	GB.Free ((void**)&np);

END_METHOD


BEGIN_PROPERTY(CDRAW_clip_x)

	CHECK_PAINTER();
	GB.ReturnInteger(dr->clipX());
	
END_PROPERTY


BEGIN_PROPERTY(CDRAW_clip_y)

	CHECK_PAINTER();
	GB.ReturnInteger(dr->clipY());

END_PROPERTY


BEGIN_PROPERTY(CDRAW_clip_w)

	CHECK_PAINTER();
	GB.ReturnInteger(dr->clipWidth());
	
END_PROPERTY


BEGIN_PROPERTY(CDRAW_clip_h)

	CHECK_PAINTER();
	GB.ReturnInteger(dr->clipHeight());

END_PROPERTY


BEGIN_PROPERTY(CDRAW_clip_enabled)

	CHECK_PAINTER();
	
	if (READ_PROPERTY) { GB.ReturnBoolean(dr->clipEnabled()); return; }
	dr->setClipEnabled(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_METHOD(CDRAW_clip, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h)

	CHECK_PAINTER();
	dr->startClip(VARG(x),VARG(y),VARG(w),VARG(h));

END_PROPERTY


GB_DESC CDrawClipDesc[] =
{
  GB_DECLARE(".DrawClip", 0), GB_VIRTUAL_CLASS(),

  GB_STATIC_PROPERTY_READ("X", "i", CDRAW_clip_x),
  GB_STATIC_PROPERTY_READ("Y", "i", CDRAW_clip_y),
  GB_STATIC_PROPERTY_READ("W", "i", CDRAW_clip_w),
  GB_STATIC_PROPERTY_READ("H", "i", CDRAW_clip_h),
  GB_STATIC_PROPERTY_READ("Width", "i", CDRAW_clip_w),
  GB_STATIC_PROPERTY_READ("Height", "i", CDRAW_clip_h),

  GB_STATIC_PROPERTY("Enabled", "b", CDRAW_clip_enabled),
  GB_STATIC_METHOD("_call", NULL, CDRAW_clip, "(X)i(Y)i(Width)i(Height)i"),

  GB_END_DECLARE
};

GB_DESC CDrawDesc[] =
{
  GB_DECLARE("Draw", 0), GB_VIRTUAL_CLASS(),

  GB_STATIC_METHOD("_init", NULL, CDRAW_init, NULL),
  GB_STATIC_METHOD("_exit", NULL, CDRAW_exit, NULL),

  GB_STATIC_METHOD("Begin", NULL, CDRAW_begin, "(Device)o"),
  GB_STATIC_METHOD("End", NULL, CDRAW_end, NULL),
  GB_STATIC_PROPERTY_READ("Device", "o", CDRAW_device),

  GB_STATIC_PROPERTY_READ("W", "i", CDRAW_width),
  GB_STATIC_PROPERTY_READ("H", "i", CDRAW_height),
  GB_STATIC_PROPERTY_READ("Width", "i", CDRAW_width),
  GB_STATIC_PROPERTY_READ("Height", "i", CDRAW_height),
  GB_STATIC_PROPERTY_READ("Resolution", "i", CDRAW_resolution),

  GB_STATIC_PROPERTY_SELF("Clip", ".DrawClip"),

  GB_STATIC_PROPERTY("BackColor", "i", CDRAW_background),
  GB_STATIC_PROPERTY("Background", "i", CDRAW_background),
  GB_STATIC_PROPERTY("Transparent", "b", CDRAW_transparent),
  GB_STATIC_PROPERTY("Invert", "b", CDRAW_invert),

  GB_STATIC_PROPERTY("ForeColor", "i", CDRAW_foreground),
  GB_STATIC_PROPERTY("Foreground", "i", CDRAW_foreground),

  GB_STATIC_PROPERTY("LineWidth", "i", CDRAW_line_width),
  GB_STATIC_PROPERTY("LineStyle", "i", CDRAW_line_style),

  GB_STATIC_PROPERTY("FillColor", "i", CDRAW_fill_color),
  GB_STATIC_PROPERTY("FillStyle", "i", CDRAW_fill_style),

  GB_STATIC_PROPERTY("Font", "Font", CDRAW_font),

  GB_STATIC_PROPERTY("FillX", "i", CDRAW_fill_x),
  GB_STATIC_PROPERTY("FillY", "i", CDRAW_fill_y),

  GB_STATIC_METHOD("Ellipse", NULL, CDRAW_ellipse, "(X)i(Y)i(Width)i(Height)i[(Start)f(Length)f]"),
  GB_STATIC_METHOD("Line", NULL, CDRAW_line, "(X1)i(Y1)i(X2)i(Y2)i"),

  GB_STATIC_METHOD("Picture", NULL, CDRAW_picture, "(Picture)Picture;(X)i(Y)i[(SrcX)i(SrcY)i(SrcWidth)i(SrcHeight)i]"),
  GB_STATIC_METHOD("Image", NULL, CDRAW_image, "(Image)Image;(X)i(Y)i[(SrcX)i(SrcY)i(SrcWidth)i(SrcHeight)i]"),
  GB_STATIC_METHOD("Zoom", NULL, CDRAW_zoom, "(Image)Image;(Zoom)i(X)i(Y)i[(SrcX)i(SrcY)i(SrcWidth)i(SrcHeight)i]"),
  GB_STATIC_METHOD("Drawing", NULL, CDRAW_drawing, "(Image)Drawing;(X)i(Y)i[(Width)i(Height)i]"),

  GB_STATIC_METHOD("Tile", NULL, CDRAW_tile, "(Picture)Picture;(X)i(Y)i(Width)i(Height)i"),

  GB_STATIC_METHOD("Point", NULL, CDRAW_point, "(X)i(Y)i"),
  GB_STATIC_METHOD("Rect", NULL, CDRAW_rect, "(X)i(Y)i(Width)i(Height)i"),
  GB_STATIC_METHOD("Text", NULL, CDRAW_text, "(Text)s(X)i(Y)i[(Width)i(Height)i(Alignment)i)]"),

  GB_STATIC_METHOD("TextWidth", "i", CDRAW_text_width, "(Text)s"),
  GB_STATIC_METHOD("TextHeight", "i", CDRAW_text_height, "(Text)s"),

  GB_STATIC_METHOD("Polyline", NULL, CDRAW_polyline, "(Points)Integer[]"),
  GB_STATIC_METHOD("Polygon", NULL, CDRAW_polygon, "(Points)Integer[]"),

  GB_END_DECLARE
};

