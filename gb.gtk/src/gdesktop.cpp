/***************************************************************************

	gdesktop.cpp

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

#include "widgets.h"
#include "gb.form.font.h"

#ifndef GTK3
#include "x11.h"
#endif

#include "gapplication.h"
#include "gmainwindow.h"
#include "gdesktop.h"

/***********************************************************************

Desktop

************************************************************************/

bool gDesktop::_colors_valid = false;
gColor gDesktop::_colors[NUM_COLORS];
gFont *gDesktop::_desktop_font = NULL;
int gDesktop::_desktop_scale = 0;
#ifdef GTK3
GtkStyleProvider *gDesktop::_css = NULL;
#endif

bool gDesktop::rightToLeft()
{
	return MAIN_rtl; //gtk_widget_get_default_direction() == GTK_TEXT_DIR_RTL;
}

void gDesktop::init()
{
	_desktop_font = new gFont();
	_desktop_font->setAll(true);
	_desktop_scale = 0;
}

void gDesktop::exit()
{
	gFont::assign(&_desktop_font);
}

gFont* gDesktop::font()
{
	if (!_desktop_font) gDesktop::init();
	return _desktop_font;
}

#ifndef GTK3
static void cb_update_font(gControl *control)
{
	control->updateFont();
}
#endif

void gDesktop::setFont(gFont *ft)
{
	gFont::set(&_desktop_font, ft ? ft->copy() : new gFont());
	_desktop_scale = 0;

#ifndef GTK3
	
	gApplication::forEachControl(cb_update_font);
	
#else

	GString *css = NULL;

	if (ft)
	{
		css = g_string_new(NULL);
		g_string_append(css, "* {\n");
		gt_css_add_font(css, _desktop_font);
		g_string_append(css, "}");
	}
	
	gt_define_style_sheet(&_css, css);

#endif
}

gMainWindow* gDesktop::activeWindow()
{
	return gMainWindow::_active ? gMainWindow::_active->topLevel() : NULL;
}

int gDesktop::width()
{
#if GTK_CHECK_VERSION(3, 22, 0)
	GdkRectangle rect;
	gdk_monitor_get_geometry(gdk_display_get_primary_monitor(gdk_display_get_default()), &rect);
	return rect.width;
#else
	return gdk_screen_get_width(gdk_screen_get_default ());
#endif
}

int gDesktop::height()
{
#if GTK_CHECK_VERSION(3, 22, 0)
	GdkRectangle rect;
	gdk_monitor_get_geometry(gdk_display_get_primary_monitor(gdk_display_get_default()), &rect);
	return rect.height;
#else
	return gdk_screen_get_height(gdk_screen_get_default());
#endif
}

int gDesktop::resolution()
{
	gdouble res = gdk_screen_get_resolution(gdk_screen_get_default());
	if (res == -1)
		res = 96;
	return res;
}

int gDesktop::scale()
{
	if (!_desktop_scale)
	{
		gFont *ft = font();
		_desktop_scale = GET_DESKTOP_SCALE(ft->size(), resolution());
	}

	return _desktop_scale;
}

gPicture* gDesktop::screenshot(int x, int y, int w, int h)
{
	return gt_grab_window(gdk_get_default_root_window(), x, y, w, h);
}

int gDesktop::count()
{
#if GTK_CHECK_VERSION(3, 22, 0)
	return gdk_display_get_n_monitors(gdk_display_get_default());
#elif defined(GTK3)
	return gdk_screen_get_n_monitors(gdk_screen_get_default());
#else
	return gdk_display_get_n_screens(gdk_display_get_default());
#endif
}

void gDesktop::geometry(int screen, GdkRectangle *rect)
{
	rect->x = rect->y = rect->width = rect->height = 0;
	if (screen < 0 || screen >= count())
		return;

#if GTK_CHECK_VERSION(3, 22, 0)
	gdk_monitor_get_geometry(gdk_display_get_monitor(gdk_display_get_default(), screen), rect);
#elif defined(GTK3)
	gdk_screen_get_monitor_geometry(gdk_screen_get_default(), screen, rect);
#else
	rect->width = gdk_screen_get_width(gdk_display_get_screen(gdk_display_get_default(), screen));
	rect->height = gdk_screen_get_height(gdk_display_get_screen(gdk_display_get_default(), screen));
#endif
}

void gDesktop::availableGeometry(int screen, GdkRectangle *rect)
{
	rect->x = rect->y = rect->width = rect->height = 0;
	if (screen < 0 || screen >= count())
		return;

#if GTK_CHECK_VERSION(3, 22, 0)
	gdk_monitor_get_workarea(gdk_display_get_monitor(gdk_display_get_default(), screen), rect);
#elif defined(GTK3)
	gdk_screen_get_monitor_workarea(gdk_screen_get_default(), screen, rect);
#else
	if (X11_get_available_geometry(screen, &rect->x, &rect->y, &rect->width, &rect->height))
		geometry(screen, rect);
#endif
}

void gDesktop::onThemeChange()
{
	gt_on_theme_change();
	_colors_valid = false;
}

#ifdef GTK3

static gColor get_color(GType type, bool fg, GtkStateFlags state)
{
	GtkStyleContext *style = gt_get_style(type, state == STATE_SELECTED ? "selection" : NULL, (type == GTK_TYPE_TOOLTIP && !fg) ? GTK_STYLE_CLASS_BACKGROUND : NULL);
	
	gtk_style_context_set_state(style, state);
	
	if (!fg)
	{
		cairo_surface_t *image;
		cairo_t *cairo;
		uchar *p;
		
		image = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 32, 32);
		cairo = cairo_create(image);
		gtk_render_background(style, cairo, 0, 0, 32, 32);
		cairo_destroy(cairo);
		
		p = (uchar *)cairo_image_surface_get_data(image) + sizeof(uint) * (16 * 33);
		return gt_rgba_to_color(p[2], p[1], p[0], p[3]);
	}
	else
	{
		GdkRGBA rgba;
		gtk_style_context_get_color(style, state, &rgba);
		return gt_to_color(&rgba);
	}
	
	if (state == STATE_SELECTED)
		g_object_unref(G_OBJECT(style));
}

#else

static gColor get_color(GType type, bool fg, GtkStateType state)
{
	GtkStyle *st = gt_get_style(type);
	GdkColor *color;

	if (type == GTK_TYPE_ENTRY)
	{
		if (fg)
			color = &st->text[state];
		else
			color = &st->base[state];
	}
	else
	{
		if (fg)
			color = &st->fg[state];
		else
			color = &st->bg[state];
	}
	
	return gt_gdkcolor_to_color(color);
}

#endif

void gDesktop::calcColors()
{
	_colors[BACKGROUND] = get_color(GTK_TYPE_WINDOW, false, STATE_NORMAL);
	_colors[FOREGROUND] = get_color(GTK_TYPE_WINDOW, true, STATE_NORMAL);
	_colors[TEXT_BACKGROUND] = get_color(GTK_TYPE_ENTRY, false, STATE_NORMAL);
	_colors[TEXT_FOREGROUND] = get_color(GTK_TYPE_ENTRY, true, STATE_NORMAL);
	_colors[SELECTED_BACKGROUND] = get_color(GTK_TYPE_ENTRY, false, STATE_SELECTED);
	_colors[SELECTED_FOREGROUND] = get_color(GTK_TYPE_ENTRY, true, STATE_SELECTED);
	_colors[BUTTON_BACKGROUND] = get_color(GTK_TYPE_BUTTON, false, STATE_NORMAL);
	_colors[BUTTON_FOREGROUND] = get_color(GTK_TYPE_BUTTON, true, STATE_NORMAL);
	_colors[TOOLTIP_BACKGROUND] = get_color(GTK_TYPE_TOOLTIP, false, STATE_NORMAL);
	_colors[TOOLTIP_FOREGROUND] = get_color(GTK_TYPE_TOOLTIP, true, STATE_NORMAL);
	#ifdef GTK3
		#if GTK_CHECK_VERSION(3, 12, 0)
			_colors[LINK_FOREGROUND] = get_color(GTK_TYPE_LINK_BUTTON, true, STATE_LINK);
			_colors[VISITED_FOREGROUND] = get_color(GTK_TYPE_LINK_BUTTON, true, (STATE_T)((int)STATE_LINK + (int)STATE_VISITED));
		#else
			_colors[LINK_FOREGROUND] = get_color(GTK_TYPE_LINK_BUTTON, true, STATE_NORMAL);
			_colors[VISITED_FOREGROUND] = IMAGE.DarkerColor(_colors[LINK_FOREGROUND]);
		#endif
	#else
		_colors[LINK_FOREGROUND] = IMAGE.LighterColor(_colors[SELECTED_BACKGROUND]);
		_colors[VISITED_FOREGROUND] = IMAGE.DarkerColor(_colors[LINK_FOREGROUND]);
	#endif
	_colors[LIGHT_BACKGROUND] = IMAGE.MergeColor(_colors[SELECTED_BACKGROUND], _colors[SELECTED_FOREGROUND], 0.3);
	_colors[LIGHT_FOREGROUND] = IMAGE.MergeColor(_colors[BACKGROUND], _colors[FOREGROUND], 0.3);
	
	_colors_valid = true;
}

gColor gDesktop::getColor(int color)
{
	if (!_colors_valid)
		calcColors();
	
	return _colors[color];
}

void gDesktop::screenResolution(int screen, double *x, double *y)
{
	GdkRectangle rect;
	
	if (screen < 0 || screen >= count())
	{
		if (*x) *x = 0;
		if (*y) *y = 0;
		return;
	}

#if GTK_CHECK_VERSION(3, 22, 0)
	GdkMonitor *monitor = gdk_display_get_monitor(gdk_display_get_default(), screen);
	gdk_monitor_get_geometry(monitor, &rect);
	if (x) *x = rect.width / (gdk_monitor_get_width_mm(monitor) / 25.4);
	if (y) *y = rect.height / (gdk_monitor_get_height_mm(monitor) / 25.4);
#else
	gdk_screen_get_monitor_geometry(gdk_screen_get_default(), screen, &rect);
	if (x) *x = rect.width / (gdk_screen_get_monitor_width_mm(gdk_screen_get_default(), screen) / 25.4);
	if (y) *y = rect.height / (gdk_screen_get_monitor_height_mm(gdk_screen_get_default(), screen) / 25.4);
#endif
}
