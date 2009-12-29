/***************************************************************************

	cpaint_impl.cpp

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

#define __CPAINT_IMPL_CPP

#include <cairo.h>
#include <cairo-xlib.h>

#include "gambas.h"
#include "gb_common.h"
#include "widgets.h"

#include "CWindow.h"
#include "CDrawingArea.h"
#include "CPicture.h"
#include "CImage.h"
#include "CFont.h"
#include "CDraw.h"
#include "cpaint_impl.h"

typedef
	struct {
		cairo_t *context;
		CFONT *font;
		}
	GB_PAINT_EXTRA;

#define EXTRA(d) ((GB_PAINT_EXTRA *)d->extra)
#define CONTEXT(d) EXTRA(d)->context

static bool init_painting(GB_PAINT *d, cairo_surface_t *target, int w, int h)
{
	d->width = w;
	d->height = h;
	d->resolutionX = 96; //device->physicalDpiX();
	d->resolutionY = 96; //device->physicalDpiY();
	
	/*if (device->paintingActive())
	{
		GB.Error("Device already being painted");
		return TRUE;
	}*/
	
	EXTRA(d)->context = cairo_create(target);
	cairo_surface_destroy(target);
	EXTRA(d)->font = NULL;
	
	return FALSE;
}


static int Begin(GB_PAINT *d)
{
	void *device = d->device;
	cairo_surface_t *target = NULL;
	int w, h;
	
	if (GB.Is(device, CLASS_Picture))
	{
		gPicture *picture = ((CPICTURE *)device)->picture;
		GdkDrawable *pixmap;
		
		if (picture->isVoid())
		{
			GB.Error("Bad picture");
			return TRUE;
		}
		
		pixmap = (GdkDrawable *)picture->getPixmap();
		w = picture->width();
		h = picture->height();
		
		target = 
			cairo_xlib_surface_create(gdk_x11_drawable_get_xdisplay(pixmap), gdk_x11_drawable_get_xid(pixmap), 
				gdk_x11_visual_get_xvisual(gdk_drawable_get_visual(pixmap)), w, h);
	}
	else if (GB.Is(device, CLASS_Image))
	{
		gPicture *picture = CIMAGE_get(((CIMAGE *)device));
		GdkPixbuf *pixbuf;
		
		if (picture->isVoid())
		{
			GB.Error("Bad picture");
			return TRUE;
		}
		
		pixbuf = picture->getPixbuf();
		w = picture->width();
		h = picture->height();
		
		target = 
			cairo_image_surface_create_for_data(picture->data(), CAIRO_FORMAT_ARGB32, w, h, 
				cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, w));
	}
	else if (GB.Is(device, CLASS_DrawingArea))
	{
		gDrawingArea *wid = (gDrawingArea *)((CWIDGET *)device)->widget;
		GdkDrawable *dr;
		
		w = wid->width();
		h = wid->height();

		if (wid->cached())
		{
			wid->resizeCache(); // Why is it needed?
			dr = wid->buffer;
		}
		else
		{
			dr = GTK_LAYOUT(wid->widget)->bin_window; 
		}

		/*target = 
			cairo_xlib_surface_create(gdk_x11_drawable_get_xdisplay(dr), gdk_x11_drawable_get_xid(dr), 
				gdk_x11_visual_get_xvisual(gdk_drawable_get_visual(dr)), w, h);*/
		
		d->width = w;
		d->height = h;
		d->resolutionX = 96; //device->physicalDpiX();
		d->resolutionY = 96; //device->physicalDpiY();
		
		EXTRA(d)->context = gdk_cairo_create(dr);
		EXTRA(d)->font = NULL;
		
		return FALSE;
	}
	
	return init_painting(d, target, w, h);
}

static void End(GB_PAINT *d)
{
	void *device = d->device;

	GB.Unref(POINTER(&EXTRA(d)->font));
	cairo_destroy(EXTRA(d)->context);

	if (GB.Is(device, CLASS_DrawingArea))
	{
		gDrawingArea *wid = (gDrawingArea *)((CWIDGET *)device)->widget;
		if (wid->cached())
			wid->setCache();
	}
}

static void Save(GB_PAINT *d)
{
	cairo_save(CONTEXT(d));
}

static void Restore(GB_PAINT *d)
{
	cairo_restore(CONTEXT(d));
}

// Font is used by X11!
static void Paint_Font(GB_PAINT *d, int set, GB_FONT *font)
{
	if (!EXTRA(d)->font)
	{
		EXTRA(d)->font = CFONT_create(new gFont());
		GB.Ref(EXTRA(d)->font);
	}
		
	if (set)
	{
		GB.Ref(*font);
		GB.Unref(POINTER(&EXTRA(d)->font));
		EXTRA(d)->font = (CFONT *)(*font);
	}
	else
		*font = (GB_FONT)EXTRA(d)->font;
}

/*static void init_path(GB_PAINT *d)
{
	switch (EXTRA(d)->fillRule)
	{
		case GB_PAINT_FILL_RULE_WINDING:
			PATH(d)->setFillRule(Qt::WindingFill); 
			break;
		case GB_PAINT_FILL_RULE_EVEN_ODD: 
		default:
			PATH(d)->setFillRule(Qt::OddEvenFill);
	}
}*/

static void Clip(GB_PAINT *d, int preserve)
{
	if (preserve)
		cairo_clip_preserve(CONTEXT(d));
	else
		cairo_clip(CONTEXT(d));
}

static void ResetClip(GB_PAINT *d)
{
	cairo_reset_clip(CONTEXT(d));
}

static void ClipExtents(GB_PAINT *d, GB_EXTENTS *ext)
{
	double x1, y1, x2, y2;
	cairo_clip_extents(CONTEXT(d), &x1, &y1, &x2, &y2);
	
	ext->x1 = (float)x1;
	ext->y1 = (float)y1;
	ext->x2 = (float)x2;
	ext->y2 = (float)y2;
}
	
static void Fill(GB_PAINT *d, int preserve)
{
	if (preserve)
		cairo_fill_preserve(CONTEXT(d));
	else
		cairo_fill(CONTEXT(d));
}

static void Stroke(GB_PAINT *d, int preserve)
{
	if (preserve)
		cairo_stroke_preserve(CONTEXT(d));
	else
		cairo_stroke(CONTEXT(d));
}
		
static void PathExtents(GB_PAINT *d, GB_EXTENTS *ext)
{
	double x1, y1, x2, y2;
	cairo_path_extents(CONTEXT(d), &x1, &y1, &x2, &y2);
	
	ext->x1 = (float)x1;
	ext->y1 = (float)y1;
	ext->x2 = (float)x2;
	ext->y2 = (float)y2;
}

static int PathContains(GB_PAINT *d, float x, float y)
{
	return cairo_in_fill(CONTEXT(d), (double)x, (double)y);
}

static void Dash(GB_PAINT *d, int set, float **dashes, int *count)
{
	int i;
	
	if (set)
	{
		if (!*dashes || *count == 0)
			cairo_set_dash(CONTEXT(d), NULL, 0, 0.0);
		else
		{
			double dd[*count];
			
			for (i = 0; i < *count; i++)
				dd[i] = (*dashes)[i];
			
			cairo_set_dash(CONTEXT(d), dd, *count, 0.0);
		}
	}
	else
	{
		*count = cairo_get_dash_count(CONTEXT(d));
		
		if (*count)
		{
			double dd[*count];
			cairo_get_dash(CONTEXT(d), dd, NULL);
			
			GB.Alloc(POINTER(dashes), sizeof(float) * *count);
			for (int i = 0; i < *count; i++)
				(*dashes)[i] = (float)dd[i];
		}
		else
		{
			*dashes = NULL;
		}
	}
}

static void DashOffset(GB_PAINT *d, int set, float *offset)
{
	if (set)
	{
		int count = cairo_get_dash_count(CONTEXT(d));
		double dashes[count];
		cairo_get_dash(CONTEXT(d), dashes, NULL);
		cairo_set_dash(CONTEXT(d), dashes, count, (double)*offset);
	}
	else
	{
		double v;
		cairo_get_dash(CONTEXT(d), NULL, &v);
		*offset = (float)v;
	}
}

		
static void FillRule(GB_PAINT *d, int set, int *value)
{
	if (set)
	{
		cairo_fill_rule_t v;
	
		switch(*value)
		{
			case GB_PAINT_FILL_RULE_EVEN_ODD: v = CAIRO_FILL_RULE_EVEN_ODD; break;
			case GB_PAINT_FILL_RULE_WINDING: default: v = CAIRO_FILL_RULE_WINDING;
		}
		
		cairo_set_fill_rule(CONTEXT(d), v);
	}
	else
	{
		switch(cairo_get_fill_rule(CONTEXT(d)))
		{
			case CAIRO_FILL_RULE_EVEN_ODD: *value = GB_PAINT_FILL_RULE_EVEN_ODD; break;
			case CAIRO_FILL_RULE_WINDING: default: *value = GB_PAINT_FILL_RULE_WINDING;
		}
	}
}

static void LineCap(GB_PAINT *d, int set, int *value)
{
	if (set)
	{
		cairo_line_cap_t v;
		
		switch (*value)
		{
			case GB_PAINT_LINE_CAP_ROUND: v = CAIRO_LINE_CAP_ROUND; break;
			case GB_PAINT_LINE_CAP_SQUARE: v = CAIRO_LINE_CAP_SQUARE; break;
			case GB_PAINT_LINE_CAP_BUTT: default: v = CAIRO_LINE_CAP_BUTT;
		}
		
		cairo_set_line_cap(CONTEXT(d), v);
	}
	else
	{
		switch (cairo_get_line_cap(CONTEXT(d)))
		{
			case CAIRO_LINE_CAP_ROUND: *value = GB_PAINT_LINE_CAP_ROUND; break;
			case CAIRO_LINE_CAP_SQUARE: *value = GB_PAINT_LINE_CAP_SQUARE; break;
			case CAIRO_LINE_CAP_BUTT: default: *value = GB_PAINT_LINE_CAP_BUTT;
		}
	}
}

static void LineJoin(GB_PAINT *d, int set, int *value)
{
	if (set)
	{
		cairo_line_join_t v;
		
		switch (*value)
		{
			case GB_PAINT_LINE_JOIN_ROUND: v = CAIRO_LINE_JOIN_ROUND; break;
			case GB_PAINT_LINE_JOIN_BEVEL: v = CAIRO_LINE_JOIN_BEVEL; break;
			case GB_PAINT_LINE_JOIN_MITER: default: v = CAIRO_LINE_JOIN_MITER;
		}
		
		cairo_set_line_join(CONTEXT(d), v);
	}
	else
	{
		switch (cairo_get_line_join(CONTEXT(d)))
		{
			case CAIRO_LINE_JOIN_ROUND: *value = GB_PAINT_LINE_JOIN_ROUND; break;
			case CAIRO_LINE_JOIN_BEVEL: *value = GB_PAINT_LINE_JOIN_BEVEL; break;
			case CAIRO_LINE_JOIN_MITER: default: *value = GB_PAINT_LINE_JOIN_MITER;
		}
	}
}

static void LineWidth(GB_PAINT *d, int set, float *value)
{
	if (set)
		cairo_set_line_width(CONTEXT(d), (double)*value);
	else
		*value = (float)cairo_get_line_width(CONTEXT(d));
}

static void MiterLimit(GB_PAINT *d, int set, float *value)
{
	if (set)
		cairo_set_miter_limit(CONTEXT(d), (double)*value);
	else
		*value = (float)cairo_get_miter_limit(CONTEXT(d));
}


static void Operator(GB_PAINT *d, int set, int *value)
{
	if (set)
	{
		cairo_operator_t v;
		
		switch (*value)
		{
			case GB_PAINT_OPERATOR_CLEAR: v = CAIRO_OPERATOR_CLEAR; break;
			case GB_PAINT_OPERATOR_SOURCE: v = CAIRO_OPERATOR_SOURCE; break;
			case GB_PAINT_OPERATOR_IN: v = CAIRO_OPERATOR_IN; break;
			case GB_PAINT_OPERATOR_OUT: v = CAIRO_OPERATOR_OUT; break;
			case GB_PAINT_OPERATOR_ATOP: v = CAIRO_OPERATOR_ATOP; break;
			case GB_PAINT_OPERATOR_DEST: v = CAIRO_OPERATOR_DEST; break;
			case GB_PAINT_OPERATOR_DEST_OVER: v = CAIRO_OPERATOR_DEST_OVER; break;
			case GB_PAINT_OPERATOR_DEST_IN: v = CAIRO_OPERATOR_DEST_IN; break;
			case GB_PAINT_OPERATOR_DEST_OUT: v = CAIRO_OPERATOR_DEST_OUT; break;
			case GB_PAINT_OPERATOR_DEST_ATOP: v = CAIRO_OPERATOR_DEST_ATOP; break;
			case GB_PAINT_OPERATOR_XOR: v = CAIRO_OPERATOR_XOR; break;
			case GB_PAINT_OPERATOR_ADD: v = CAIRO_OPERATOR_ADD; break;
			case GB_PAINT_OPERATOR_SATURATE: v = CAIRO_OPERATOR_SATURATE; break;
			case GB_PAINT_OPERATOR_OVER: default: v = CAIRO_OPERATOR_OVER; break;
		}
		
		cairo_set_operator(CONTEXT(d), v);
	}
	else
	{
		switch (cairo_get_operator(CONTEXT(d)))
		{
			case CAIRO_OPERATOR_CLEAR: *value = GB_PAINT_OPERATOR_CLEAR; break;
			case CAIRO_OPERATOR_SOURCE: *value = GB_PAINT_OPERATOR_SOURCE; break;
			case CAIRO_OPERATOR_IN: *value = GB_PAINT_OPERATOR_IN; break;
			case CAIRO_OPERATOR_OUT: *value = GB_PAINT_OPERATOR_OUT; break;
			case CAIRO_OPERATOR_ATOP: *value = GB_PAINT_OPERATOR_ATOP; break;
			case CAIRO_OPERATOR_DEST: *value = GB_PAINT_OPERATOR_DEST; break;
			case CAIRO_OPERATOR_DEST_OVER: *value = GB_PAINT_OPERATOR_DEST_OVER; break;
			case CAIRO_OPERATOR_DEST_IN: *value = GB_PAINT_OPERATOR_DEST_IN; break;
			case CAIRO_OPERATOR_DEST_OUT: *value = GB_PAINT_OPERATOR_DEST_OUT; break;
			case CAIRO_OPERATOR_DEST_ATOP: *value = GB_PAINT_OPERATOR_DEST_ATOP; break;
			case CAIRO_OPERATOR_XOR: *value = GB_PAINT_OPERATOR_XOR; break;
			case CAIRO_OPERATOR_ADD: *value = GB_PAINT_OPERATOR_ADD; break;
			case CAIRO_OPERATOR_SATURATE: *value = GB_PAINT_OPERATOR_SATURATE; break;
			case CAIRO_OPERATOR_OVER: default: *value = GB_PAINT_OPERATOR_OVER;
		}
	}
}


static void NewPath(GB_PAINT *d)
{
	cairo_new_path(CONTEXT(d));
}

static void ClosePath(GB_PAINT *d)
{
	cairo_close_path(CONTEXT(d));
}

		
static void Arc(GB_PAINT *d, float xc, float yc, float radius, float angle, float length)
{
	cairo_new_sub_path(CONTEXT(d));
	if (length >= 0.0)
		cairo_arc(CONTEXT(d), xc, yc, radius, angle, angle + length);
	else
		cairo_arc_negative(CONTEXT(d), xc, yc, radius, angle, angle + length);
}

static void Rectangle(GB_PAINT *d, float x, float y, float width, float height)
{
	cairo_rectangle(CONTEXT(d), x, y, width, height);
}

static void GetCurrentPoint(GB_PAINT *d, float *x, float *y)
{
	double cx, cy;
	
	cairo_get_current_point(CONTEXT(d), &cx, &cy);
	*x = (float)cx;
	*y = (float)cy;
}

static void MoveTo(GB_PAINT *d, float x, float y)
{
	cairo_move_to(CONTEXT(d), x, y);
}

static void LineTo(GB_PAINT *d, float x, float y)
{
	cairo_line_to(CONTEXT(d), x, y);
}

static void CurveTo(GB_PAINT *d, float x1, float y1, float x2, float y2, float x3, float y3)
{
	cairo_curve_to(CONTEXT(d), x1, y1, x2, y2, x3, y3);
}

static void Text(GB_PAINT *d, const char *text, int len)
{
}

static void TextExtents(GB_PAINT *d, const char *text, int len, GB_EXTENTS *ext)
{
}

		
static void Matrix(GB_PAINT *d, int set, GB_TRANSFORM matrix)
{
	cairo_matrix_t *t = (cairo_matrix_t *)matrix;
	
	if (set)
	{
		if (t)
			cairo_set_matrix(CONTEXT(d), t);
		else
			cairo_identity_matrix(CONTEXT(d));
	}
	else
		cairo_get_matrix(CONTEXT(d), t);
}

		
static void SetBrush(GB_PAINT *d, GB_BRUSH brush)
{
	cairo_set_source(CONTEXT(d), (cairo_pattern_t *)brush);
}

		
static void BrushFree(GB_BRUSH brush)
{
	// Should I release the surface associated with an image brush?
	cairo_pattern_destroy((cairo_pattern_t *)brush);
}

static void BrushColor(GB_BRUSH *brush, GB_COLOR color)
{
	double r, g, b, a;
	
	a = ((color >> 24) ^ 0xFF) / 255.0;
	r = ((color >> 16) & 0xFF) / 255.0;
	g = ((color >> 8) & 0xFF) / 255.0;
	b = (color & 0xFF) / 255.0;
	
	*brush = (GB_BRUSH)cairo_pattern_create_rgba(r, g, b, a);
}

static void BrushImage(GB_BRUSH *brush, GB_IMAGE image)
{
	gPicture *picture = CIMAGE_get((CIMAGE *)image);
	cairo_surface_t *surface;

	picture->getPixbuf();
	
	surface =	cairo_image_surface_create_for_data(picture->data(), CAIRO_FORMAT_ARGB32, picture->width(), picture->height(), 
		cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, picture->width()));
				
	*brush = (GB_BRUSH)cairo_pattern_create_for_surface(surface);
}

static void BrushLinearGradient(GB_BRUSH *brush, float x0, float y0, float x1, float y1, int nstop, double *positions, GB_COLOR *colors, int extend)
{
	/*
	QLinearGradient gradient;
	int i;
	
	gradient.setStart((qreal)x0, (qreal)y0);
	gradient.setFinalStop((qreal)x1, (qreal)y1);
	
	for (i = 0; i < nstop; i++)
		gradient.setColorAt((qreal)positions[i], CCOLOR_make(colors[i]));

	switch (extend)
	{
		case GB_PAINT_EXTEND_REPEAT:
			gradient.setSpread(QGradient::RepeatSpread); break;
		case GB_PAINT_EXTEND_REFLECT:
			gradient.setSpread(QGradient::ReflectSpread); break;
		case GB_PAINT_EXTEND_PAD:
		default:
			gradient.setSpread(QGradient::PadSpread);
	}

	QBrush *br = new QBrush(gradient);
	*brush = br;
	*/
}

static void BrushRadialGradient(GB_BRUSH *brush, float cx0, float cy0, float r0, float cx1, float cy1, float r1, int nstop, double *positions, GB_COLOR *colors, int extend)
{
	/*
	QRadialGradient gradient;
	int i;
	
	gradient.setCenter((qreal)cx0, (qreal)cy0);
	gradient.setRadius((qreal)r0);
	gradient.setFocalPoint((qreal)cx1, (qreal)cy1);
	
	for (i = 0; i < nstop; i++)
		gradient.setColorAt((qreal)positions[i], CCOLOR_make(colors[i]));

	switch (extend)
	{
		case GB_PAINT_EXTEND_REPEAT:
			gradient.setSpread(QGradient::RepeatSpread); break;
		case GB_PAINT_EXTEND_REFLECT:
			gradient.setSpread(QGradient::ReflectSpread); break;
		case GB_PAINT_EXTEND_PAD:
		default:
			gradient.setSpread(QGradient::PadSpread);
	}

	QBrush *br = new QBrush(gradient);
	*brush = br;
	*/
}

static void BrushMatrix(GB_BRUSH brush, int set, GB_TRANSFORM matrix)
{
	cairo_matrix_t *t = (cairo_matrix_t *)matrix;
	cairo_pattern_t *pattern = (cairo_pattern_t *)brush;
	
	if (set)
	{
		if (t)
			cairo_pattern_set_matrix(pattern, t);
		else
		{
			cairo_matrix_t identity;
			cairo_matrix_init_identity(&identity);
			cairo_pattern_set_matrix(pattern, &identity);
		}
	}
	else
		cairo_pattern_get_matrix(pattern, t);
}

static void TransformCreate(GB_TRANSFORM *matrix)
{
	GB.Alloc(POINTER(matrix), sizeof(cairo_matrix_t));
	cairo_matrix_init_identity((cairo_matrix_t *)*matrix);
}

static void TransformDelete(GB_TRANSFORM *matrix)
{
	GB.Free(POINTER(matrix));
}

static void TransformInit(GB_TRANSFORM matrix, float xx, float yx, float xy, float yy, float x0, float y0)
{
	cairo_matrix_init((cairo_matrix_t *)matrix, xx, yx, xy, yy, x0, y0);
}

static void TransformTranslate(GB_TRANSFORM matrix, float tx, float ty)
{
	cairo_matrix_translate((cairo_matrix_t *)matrix, tx, ty);
}

static void TransformScale(GB_TRANSFORM matrix, float sx, float sy)
{
	cairo_matrix_scale((cairo_matrix_t *)matrix, sx, sy);
}

static void TransformRotate(GB_TRANSFORM matrix, float angle)
{
	cairo_matrix_rotate((cairo_matrix_t *)matrix, angle);
}

static int TransformInvert(GB_TRANSFORM matrix)
{
	cairo_status_t status = cairo_matrix_invert((cairo_matrix_t *)matrix);
	return status != CAIRO_STATUS_SUCCESS;
}

static void TransformMultiply(GB_TRANSFORM matrix, GB_TRANSFORM matrix2)
{
	cairo_matrix_multiply((cairo_matrix_t *)matrix, (cairo_matrix_t *)matrix, (cairo_matrix_t *)matrix2);
}


GB_PAINT_DESC PAINT_Interface = {
	sizeof(GB_PAINT_EXTRA),
	Begin,
	End,
	Save,
	Restore,
	Paint_Font,
	Clip,
	ResetClip,
	ClipExtents,
	Fill,
	Stroke,
	PathExtents,
	PathContains,
	Dash,
	DashOffset,
	FillRule,
	LineCap,
	LineJoin,
	LineWidth,
	MiterLimit,
	Operator,
	NewPath,
	ClosePath,
	Arc,
	Rectangle,
	GetCurrentPoint,
	MoveTo,
	LineTo,
	CurveTo,
	Text,
	TextExtents,
	Matrix,
	SetBrush,
	{
		BrushFree,
		BrushColor,
		BrushImage,
		BrushLinearGradient,
		BrushRadialGradient,
		BrushMatrix,
	},
	{
		TransformCreate,
		TransformDelete,
		TransformInit,
		TransformTranslate,
		TransformScale,
		TransformRotate,
		TransformInvert,
		TransformMultiply
	}
};

void PAINT_begin(void *device)
{
	DRAW.Paint.Begin(device);
}

void PAINT_end()
{
	DRAW.Paint.End();
}

void PAINT_clip(int x, int y, int w, int h)
{
	GB_PAINT *d = (GB_PAINT *)DRAW.Paint.GetCurrent();
	cairo_rectangle(CONTEXT(d), x, y, w, h);
	cairo_clip(CONTEXT(d));
}

