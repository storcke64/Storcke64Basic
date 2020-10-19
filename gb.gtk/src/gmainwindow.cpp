/***************************************************************************

  gmainwindow.cpp

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

#include <ctype.h>
#include <time.h>

#include "widgets.h"

#ifdef GDK_WINDOWING_X11
#include <X11/extensions/shape.h>
#endif

#include "x11.h"
#include "sm/sm.h"

#include "gapplication.h"
#include "gdesktop.h"
#include "gkey.h"
#include "gmenu.h"
#include "gmessage.h"
#include "gdialog.h"
#include "gmouse.h"
#include "gmainwindow.h"

//#define DEBUG_RESIZE 1

#define CHECK_STATE(_var, _state) \
	if (event->changed_mask & _state) \
	{ \
		v = (event->new_window_state & _state) != 0; \
		if (v != data->_var) \
		{ \
			data->_var = v; \
			has_changed = true; \
		} \
	}

static gboolean cb_frame(GtkWidget *widget,GdkEventWindowState *event,gMainWindow *data)
{
	bool has_changed = false;
	bool v;
	
	CHECK_STATE(_minimized, GDK_WINDOW_STATE_ICONIFIED);
	CHECK_STATE(_maximized, GDK_WINDOW_STATE_MAXIMIZED);
	CHECK_STATE(_sticky, GDK_WINDOW_STATE_STICKY);
	CHECK_STATE(_fullscreen, GDK_WINDOW_STATE_FULLSCREEN);

	if (event->changed_mask & GDK_WINDOW_STATE_ABOVE)
	{
		if (event->new_window_state & GDK_WINDOW_STATE_ABOVE)
			data->stack = 1;
		else if (data->stack == 1)
			data->stack = 0;
	}
	if (event->changed_mask & GDK_WINDOW_STATE_BELOW)
	{
		if (event->new_window_state & GDK_WINDOW_STATE_BELOW)
			data->stack = 2;
		else if (data->stack == 2)
			data->stack = 0;
	}

	if (has_changed)
	{
		#ifdef DEBUG_RESIZE
		fprintf(stderr, "cb_frame: min = %d max = %d fs = %d\n", data->_minimized, data->_maximized, data->_fullscreen);
		#endif
		data->_csd_w = data->_csd_h = -1;
		/*data->calcCsdSize();
		data->performArrange();*/
	}

	if (event->changed_mask & (GDK_WINDOW_STATE_ICONIFIED | GDK_WINDOW_STATE_MAXIMIZED | GDK_WINDOW_STATE_FULLSCREEN | GDK_WINDOW_STATE_STICKY | GDK_WINDOW_STATE_ABOVE | GDK_WINDOW_STATE_BELOW))
		data->emit(SIGNAL(data->onState));

	return false; 
}

static gboolean cb_show(GtkWidget *widget, gMainWindow *data)
{
	if (data->_grab_on_show)
	{
		data->_grab_on_show = FALSE;
		gApplication::grabPopup();
	}

	data->emitOpen();

	if (data->_opened)
	{
		data->setGeometryHints();

		data->performArrange();
		#ifdef DEBUG_RESIZE
		fprintf(stderr, "cb_show\n");
		#endif
		data->emitResize();
		data->emit(SIGNAL(data->onShow));
		data->_not_spontaneous = false;
	}
	return false;
}

static gboolean cb_map(GtkWidget *widget, GdkEvent *event, gMainWindow *data)
{
	data->_unmap = false;
	return cb_show(widget, data);
}

static gboolean cb_hide(GtkWidget *widget, gMainWindow *data)
{
	if (!data->_unmap)
	{
		data->emit(SIGNAL(data->onHide));
		data->_not_spontaneous = false;
	}

	return false;
	//if (data == gDesktop::activeWindow())
	//	gMainWindow::setActiveWindow(NULL);
}

static gboolean cb_unmap(GtkWidget *widget, GdkEvent *event, gMainWindow *data)
{
	bool ret = cb_hide(widget, data);
	data->_unmap = true;
	return ret;
}

static gboolean cb_close(GtkWidget *widget,GdkEvent *event, gMainWindow *data)
{
	if (!gMainWindow::_current || data == gMainWindow::_current)
		data->doClose();

	return true;
}

static gboolean cb_configure(GtkWidget *widget, GdkEventConfigure *event, gMainWindow *data)
{
	gint x, y, w, h;

	if (data->_opened)
	{
		if (data->isTopLevel())
		{
			gtk_window_get_position(GTK_WINDOW(data->border), &x, &y);
		}
		else
		{
			x = event->x;
			y = event->y;
		}

		#ifdef DEBUG_RESIZE
		fprintf(stderr, "cb_configure: %s: (%d %d %d %d) -> (%d/%d %d/%d %d %d) window = %p resized = %d send_event = %d\n", data->name(), data->bufX, data->bufY, data->bufW, data->bufH, x, event->x, y, event->y, event->width, event->height, event->window, data->_resized, event->send_event);
		#endif

		if (x != data->bufX || y != data->bufY)
		{
			data->bufX = x;
			data->bufY = y;
			if (data->onMove) data->onMove(data);
		}
		
		#ifdef GTK3
		//data->_csd_w = data->_csd_h = -1;
		if (data->isTopLevel())
			return false;
		#endif

		w = event->width;
		h = event->height;
		
		if ((w != data->bufW) || (h != data->bufH) || (data->_resized) || !event->window)
		{
			data->_resized = false;
			data->bufW = w;
			data->bufH = h;
			#ifdef DEBUG_RESIZE
			fprintf(stderr, "cb_configure\n");
			#endif
			data->emitResize();
		}
	}

	return false;
}

#ifdef GTK3
static void cb_resize(GtkWidget *wid, GdkRectangle *a, gMainWindow *data)
{
	int w, h;
	
	if (data->layout)
		return;
	
	w = a->width;
	h = a->height;
	
	data->calcCsdSize();
	if (data->_csd_w < 0)
		return;
	
	w -= data->_csd_w;
	h -= data->_csd_h;
	
	if (w != data->bufW || h != data->bufH || data->_resized)
	{
		#ifdef DEBUG_RESIZE
		fprintf(stderr, "cb_resize: %s: %d %d\n", data->name(), w, h);
		#endif
	
		data->_resized = false;
		data->bufW = w;
		data->bufH = h;
		data->emitResizeLater();
	}
}

static void cb_resize_layout(GtkWidget *wid, GdkRectangle *a, gMainWindow *data)
{
	int w, h;
	
	data->calcCsdSize();
	
	w = a->width;
	h = a->height;
	
	if (w != data->bufW || h != data->bufH || data->_resized)
	{
		#ifdef DEBUG_RESIZE
		fprintf(stderr, "cb_resize_layout: %s: %d x %d / resize = %d\n", data->name(), w, h, data->_resized);
		#endif

		data->_resized = false;
		data->bufW = w;
		data->bufH = h;
		data->emitResizeLater();
	}
}


static gboolean cb_draw(GtkWidget *wid, cairo_t *cr, gMainWindow *data)
{
	if (data->isTransparent())
	{
		if (data->background() == COLOR_DEFAULT)
			gt_cairo_set_source_color(cr, 0xFF000000);
		else
			gt_cairo_set_source_color(cr, data->background());
		cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
		cairo_paint(cr);
	}

	if (data->_picture)
	{
		cairo_pattern_t *pattern;

		pattern = cairo_pattern_create_for_surface(data->_picture->getSurface());
		cairo_pattern_set_extend(pattern, CAIRO_EXTEND_REPEAT);

		cairo_set_source(cr, pattern);
		cairo_paint(cr);

		cairo_pattern_destroy(pattern);
	}

	return false;
}
#else
static gboolean cb_expose(GtkWidget *wid, GdkEventExpose *e, gMainWindow *data)
{
	bool draw_bg = data->isTransparent();
	bool draw_pic = data->_picture;

	if (!draw_bg && !draw_pic)
		return false;

	cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(wid));

	if (draw_bg)
	{
		if (data->background() == COLOR_DEFAULT)
			gt_cairo_set_source_color(cr, 0xFF000000);
		else
			gt_cairo_set_source_color(cr, data->background());
		cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
		cairo_paint(cr);
	}

	if (draw_pic)
	{
		cairo_pattern_t *pattern;

		gdk_cairo_region(cr, e->region);
		cairo_clip(cr);

		pattern = cairo_pattern_create_for_surface(data->_picture->getSurface());
		cairo_pattern_set_extend(pattern, CAIRO_EXTEND_REPEAT);

		cairo_set_source(cr, pattern);
		cairo_paint(cr);

		cairo_pattern_destroy(pattern);
	}

	cairo_destroy(cr);
	return false;
}
#endif



GList *gMainWindow::windows = NULL;
gMainWindow *gMainWindow::_active = NULL;
gMainWindow *gMainWindow::_current = NULL;

void gMainWindow::initialize()
{
	//fprintf(stderr, "new window: %p in %p\n", this, parent());

	stack = 0;
	_type = 0;
	accel = NULL;
	_default = NULL;
	_cancel = NULL;
	menuBar = NULL;
	layout = NULL;
	_icon = NULL;
	_picture = NULL;
	focus = 0;
	_title = NULL;
	_current = NULL;
	_style = NULL;
	_resize_last_w = _resize_last_h = -1;
	_min_w = _min_h = 0;
	_csd_w  = _csd_h = -1;

	_opened = false;
	_sticky = false;
	_persistent = false;
	_mask = false;
	_masked = false;
	_resized = false;
	_top_only = false;
	_closing = false;
	_not_spontaneous = false;
	_skip_taskbar = false;
	_xembed = false;
	_activate = false;
	_hidden = false;
	_hideMenuBar = false;
	_showMenuBar = true;
	_initMenuBar = true;
	_popup = false;
	_maximized = _minimized = _fullscreen = false;
	_transparent = false;
	_utility = false;
	_no_take_focus = false;
	_moved = false;
	_resizable = true;
	_unmap = false;
	_grab_on_show	= false;
	
	onOpen = NULL;
	onShow = NULL;
	onHide = NULL;
	onMove = NULL;
	onResize = NULL;
	onActivate = NULL;
	onDeactivate = NULL;
	onState = NULL;
	onFontChange = NULL;

	accel = gtk_accel_group_new();
}

void gMainWindow::initWindow()
{
	//resize(200,150);

	if (!isTopLevel())
	{
		g_signal_connect(G_OBJECT(border), "configure-event", G_CALLBACK(cb_configure), (gpointer)this);
		#ifdef GTK3
		g_signal_connect_after(G_OBJECT(border), "size-allocate", G_CALLBACK(cb_resize), (gpointer)this);
		#endif
		g_signal_connect_after(G_OBJECT(border), "map", G_CALLBACK(cb_show), (gpointer)this);
		g_signal_connect(G_OBJECT(border),"unmap", G_CALLBACK(cb_hide),(gpointer)this);
		//g_signal_connect_after(G_OBJECT(border), "size-allocate", G_CALLBACK(cb_configure), (gpointer)this);
		ON_DRAW_BEFORE(widget, this, cb_expose, cb_draw);
		gtk_widget_add_events(border, GDK_STRUCTURE_MASK);
	}
	else
	{
		//g_signal_connect(G_OBJECT(border),"size-request",G_CALLBACK(cb_realize),(gpointer)this);
		//g_signal_connect(G_OBJECT(border), "show", G_CALLBACK(cb_show),(gpointer)this);
		g_signal_connect(G_OBJECT(border), "hide", G_CALLBACK(cb_hide),(gpointer)this);
		g_signal_connect(G_OBJECT(border), "map-event", G_CALLBACK(cb_map),(gpointer)this);
		g_signal_connect(G_OBJECT(border), "unmap-event", G_CALLBACK(cb_unmap),(gpointer)this);
		g_signal_connect(G_OBJECT(border), "delete-event", G_CALLBACK(cb_close),(gpointer)this);
		g_signal_connect(G_OBJECT(border), "window-state-event", G_CALLBACK(cb_frame),(gpointer)this);

		gtk_widget_add_events(widget,GDK_BUTTON_MOTION_MASK | GDK_STRUCTURE_MASK);
		ON_DRAW_BEFORE(border, this, cb_expose, cb_draw);

		g_signal_connect(G_OBJECT(border), "configure-event", G_CALLBACK(cb_configure), (gpointer)this);
		#ifdef GTK3
		g_signal_connect_after(G_OBJECT(border), "size-allocate", G_CALLBACK(cb_resize), (gpointer)this);
		#endif
	}

	gtk_window_add_accel_group(GTK_WINDOW(topLevel()->border), accel);

	have_cursor = true; //parent() == 0 && !_xembed;
}

#if 0 //def GTK3

static void (*old_fixed_get_preferred_width)(GtkWidget *, gint *, gint *);
static void (*old_fixed_get_preferred_height)(GtkWidget *, gint *, gint *);

static void gtk_fixed_get_preferred_width(GtkWidget *widget, gint *minimum_size, gint *natural_size)
{
	(*old_fixed_get_preferred_width)(widget, minimum_size, natural_size);
	*minimum_size = 0;
}

static void gtk_fixed_get_preferred_height(GtkWidget *widget, gint *minimum_size, gint *natural_size)
{
	(*old_fixed_get_preferred_height)(widget, minimum_size, natural_size);
	*minimum_size = 0;
}

#endif

gMainWindow::gMainWindow(int plug) : gContainer(NULL)
{
  initialize();
	g_typ = Type_gMainWindow;

	windows = g_list_append(windows, (gpointer)this);

	_xembed = plug != 0;

	if (_xembed)
		border = gtk_plug_new(plug);
	else
		border = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	widget = gtk_fixed_new(); //gtk_layout_new(0,0);

	realize(false);
	initWindow();

	gtk_widget_realize(border);
	gtk_widget_show(widget);
	gtk_widget_set_size_request(border, 1, 1);

	setCanFocus(true);
}

gMainWindow::gMainWindow(gContainer *par) : gContainer(par)
{
	initialize();
	g_typ = Type_gMainWindow;

#ifdef GTK3
	//border = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	border = gtk_fixed_new();
#else
	border = gtk_alignment_new(0, 0, 1, 1);
#endif
	widget = gtk_fixed_new();

	realize(false);
	initWindow();

	setCanFocus(true);
}

gMainWindow::~gMainWindow()
{
	//fprintf(stderr, "delete window %p %s\n", this, name());

	gApplication::handleFocusNow();

	if (_opened)
	{
		emit(SIGNAL(onClose));
		_opened = false;
		if (GTK_IS_WINDOW(border) && isModal())
			gApplication::exitLoop(this);
	}

	gPicture::assign(&_picture);
	gPicture::assign(&_icon);
	if (_title) g_free(_title);
	g_object_unref(accel);
	if (_style) g_object_unref(_style);

	if (_active == this)
		_active = NULL;

	if (gApplication::mainWindow() == this)
		gApplication::setMainWindow(NULL);

	windows = g_list_remove(windows, (gpointer)this);
}

int gMainWindow::getStacking()
{
	return stack;
}

void gMainWindow::setSticky(bool vl)
{
	if (!isTopLevel()) return;

	_sticky = vl;

	if (vl) 
		gtk_window_stick(GTK_WINDOW(border));
	else
		gtk_window_unstick(GTK_WINDOW(border));
}

void gMainWindow::setStacking(int vl)
{
  stack=vl;
	if (!isTopLevel()) return;

	switch (vl)
	{
		case 0:
			gtk_window_set_keep_below(GTK_WINDOW(border),FALSE);
			gtk_window_set_keep_above(GTK_WINDOW(border),FALSE);
			break;
		case 1:
			gtk_window_set_keep_below(GTK_WINDOW(border),FALSE);
			gtk_window_set_keep_above(GTK_WINDOW(border),TRUE);
			break;
		case 2:
			gtk_window_set_keep_above(GTK_WINDOW(border),FALSE);
			gtk_window_set_keep_below(GTK_WINDOW(border),TRUE);
			break;
	}
}

void gMainWindow::setRealBackground(gColor color)
{
	if (!_picture)
	{
		gControl::setRealBackground(color);
		gMenu::updateColor(this);
	}
}

void gMainWindow::setRealForeground(gColor color)
{
	gControl::setRealForeground(color);
	gMenu::updateColor(this);
}

void gMainWindow::move(int x, int y)
{
	if (isTopLevel())
	{
		if (!_moved && (x || y))
			_moved = true;

		if (x == bufX && y == bufY)
			return;

		bufX = x;
		bufY = y;

		gtk_window_move(GTK_WINDOW(border), x, y);
	}
	else
	{
		gContainer::move(x,y);
	}
}


bool gMainWindow::resize(int w, int h)
{
	if (!isTopLevel())
	{
		if (gContainer::resize(w, h))
			return true;
	}
	else
	{
		if (w == bufW && h == bufH)
			return true;

		//fprintf(stderr, "gMainWindow::resize: %d %d %s\n", w, h, name());
		//gdk_window_enable_synchronized_configure (border->window);

		bufW = w < 0 ? 0 : w;
		bufH = h < 0 ? 0 : h;

		if (w < 1 || h < 1)
		{
			if (isVisible())
				gtk_widget_hide(border);
		}
		else
		{
			if (isResizable())
				gtk_window_resize(GTK_WINDOW(border), w, h);
			else
			{
				gtk_widget_set_size_request(border, w, h);
			}

			if (isVisible())
				gtk_widget_show(border);
		}
	}

	_resized = true;
	return false;
}

/*void gMainWindow::moveResize(int x, int y, int w, int h)
{
	//if (isTopLevel())
	//	gdk_window_move_resize(border->window, x, y, w, h);
	//else
		gContainer::moveResize(x, y, w, h);
}*/

bool gMainWindow::emitOpen()
{
	//fprintf(stderr, "emit Open: %p (%d %d) %d resizable = %d fullscreen = %d\n", this, width(), height(), _opened, isResizable(), fullscreen());

	if (_opened)
		return false;
	
	_opened = true;
	_closed = false;
	//_no_resize_event = true; // If the event loop is run during emitOpen(), some spurious configure events are received.

	if (!_min_w && !_min_h)
	{
		_min_w = width();
		_min_h = height();
	}

	gtk_widget_realize(border);

	#ifdef DEBUG_RESIZE
	fprintf(stderr, "#2\n");
	#endif
	performArrange();
	emit(SIGNAL(onOpen));
	if (_closed)
	{
		_opened = false;
		return true;
	}

	//fprintf(stderr, "emit Move & Resize: %p\n", this);
	emit(SIGNAL(onMove));
	#ifdef DEBUG_RESIZE
	fprintf(stderr, "cb_open\n");
	#endif
	emitResize();

	return false;
}

void gMainWindow::present()
{
	if (_no_take_focus)
		gtk_widget_show(GTK_WIDGET(border));
	else
		gtk_window_present(GTK_WINDOW(border));
}

void gMainWindow::afterShow()
{
	if (_activate)
	{
		present();
		_activate = false;
	}
}

void gMainWindow::setTransientFor()
{
	gMainWindow *parent = _current;

	if (!parent)
		parent = gApplication::mainWindow();
		
	if (!parent)
		parent = _active;

	if (parent)
	{
		parent = parent->topLevel();
		if (parent != this)
		{
			//fprintf(stderr, "setTransientFor: %s -> %s\n", name(), parent->name());
			gtk_window_set_transient_for(GTK_WINDOW(border), GTK_WINDOW(parent->border));
		}
	}
}

void gMainWindow::setVisible(bool vl)
{
	if (!vl)
		_hidden = true;

	if (!isTopLevel())
	{
		gContainer::setVisible(vl);
		return;
	}

	if (vl == isVisible())
		return;
	
	if (vl)
	{
		//bool arr = !isVisible();

		emitOpen();
		if (!_opened)
			return;

		_not_spontaneous = !isVisible();
		_visible = true;
		_hidden = false;

		setTransparent(_transparent); // must not call gtk_window_present!

		if (isTopLevel())
		{
			/*if (!_xembed)
			{
				fprintf(stderr, "gtk_window_group_add_window: %p -> %p\n", border, gApplication::currentGroup());
				gtk_window_group_add_window(gApplication::currentGroup(), GTK_WINDOW(border));
				fprintf(stderr, "-> %p\n", gtk_window_get_group(GTK_WINDOW(border)));
			}*/

			// Thanks for Ubuntu's GTK+ patching :-(
			#ifndef GTK3
			//gtk_window_set_has_resize_grip(GTK_WINDOW(border), false);
			if (g_object_class_find_property(G_OBJECT_GET_CLASS(border), "has-resize-grip"))
				g_object_set(G_OBJECT(border), "has-resize-grip", false, (char *)NULL);
			#endif

			gtk_window_move(GTK_WINDOW(border), bufX, bufY);

			/*if (isPopup())
			{
				gtk_widget_show_now(border);
				gtk_widget_grab_focus(border);
			}
			else
			{*/
				present();
			//}

			if (!_title || !*_title)
				gtk_window_set_title(GTK_WINDOW(border), gApplication::defaultTitle());

			if (isUtility())
			{
				setTransientFor();
				if (!_no_take_focus)
					present();
			}

			if (gApplication::mainWindow() == this)
			{
				int desktop = session_manager_get_desktop();
				if (desktop >= 0)
				{
					//fprintf(stderr, "X11_window_set_desktop: %d (%d)\n", desktop, true);
					X11_window_set_desktop((Window)handle(), true, desktop);
					session_manager_set_desktop(-1);
				}
			}
		}
		else
		{
			gtk_widget_show(border);
			#ifdef DEBUG_RESIZE
			fprintf(stderr, "#3\n");
			#endif
			parent()->performArrange();
			performArrange();
		}

		drawMask();

		if (focus)
		{
			//fprintf(stderr, "focus = %s\n", focus->name());
			focus->setFocus();
			focus = 0;
		}

		if (isSkipTaskBar())
			_activate = true;

		/*if (arr)
		{
				fprintf(stderr, "#4\n");
				performArrange();
		}*/
	}
	else
	{
		if (this == _active)
			focus = gApplication::activeControl();

		_not_spontaneous = isVisible();
		gContainer::setVisible(false);

		if (_popup)
			gApplication::exitLoop(this);

		if (gApplication::_button_grab && !gApplication::_button_grab->isReallyVisible())
				gApplication::setButtonGrab(NULL);
	}
}


void gMainWindow::setMinimized(bool vl)
{
	if (!isTopLevel()) return;

	_minimized = vl;
	if (vl) gtk_window_iconify(GTK_WINDOW(border));
	else    gtk_window_deiconify(GTK_WINDOW(border));
}

void gMainWindow::setMaximized(bool vl)
{
	if (!isTopLevel())
		return;

	_maximized = vl;
	_csd_w = _csd_h = -1;

	if (vl)
		gtk_window_maximize(GTK_WINDOW(border));
	else
		gtk_window_unmaximize(GTK_WINDOW(border));
}

void gMainWindow::setFullscreen(bool vl)
{
	if (!isTopLevel())
		return;

	_fullscreen = vl;
	_csd_w = _csd_h = -1;

	if (vl)
	{
		gtk_window_fullscreen(GTK_WINDOW(border));
		if (isVisible())
			present();
	}
	else
		gtk_window_unfullscreen(GTK_WINDOW(border));
}

void gMainWindow::center()
{
	GdkRectangle rect;
	int x, y;

	if (!isTopLevel()) return;

	gDesktop::availableGeometry(screen(), &rect);

	x = rect.x + (rect.width - width()) / 2;
	y = rect.y + (rect.height - height()) / 2;

	move(x, y);
}

bool gMainWindow::isModal() const
{
	if (!isTopLevel()) return false;

	return gtk_window_get_modal(GTK_WINDOW(border));
}

void gMainWindow::showModal()
{
  gMainWindow *save;

	if (!isTopLevel()) return;
	if (isModal()) return;

	//show();
	setType(GTK_WINDOW_TOPLEVEL);

	gtk_window_set_modal(GTK_WINDOW(border), true);
  center();
	//show();
	gtk_grab_add(border);

	setTransientFor();

	save = _current;
	_current = this;

	gApplication::enterLoop(this, true);

	_current = save;

	gtk_grab_remove(border);
	gtk_window_set_modal(GTK_WINDOW(border), false);

	if (!_persistent)
		destroyNow();
	else
		hide();
}

void gMainWindow::showPopup(int x, int y)
{
  gMainWindow *save;
	bool has_border;
	int oldx, oldy;
	GdkWindowTypeHint type;

	if (!isTopLevel()) return;
	if (isModal()) return;

	//gtk_widget_unrealize(border);
	//((GtkWindow *)border)->type = GTK_WINDOW_POPUP;
	//gtk_widget_realize(border);

	oldx = left();
	oldy = top();

	_popup = true;
	setType(GTK_WINDOW_POPUP);
	
	has_border = gtk_window_get_decorated(GTK_WINDOW(border));
	type = gtk_window_get_type_hint(GTK_WINDOW(border));

	gtk_window_set_decorated(GTK_WINDOW(border), false);
	gtk_window_set_type_hint(GTK_WINDOW(border), GDK_WINDOW_TYPE_HINT_COMBO);
	
	setTransientFor();

	gtk_window_resize(GTK_WINDOW(border), bufW, bufH);
  move(x, y);
	raise();
	setFocus();

	save = _current;
	_current = this;

	gApplication::enterPopup(this);

	_current = save;
	_popup = false;

	if (!_persistent)
	{
		destroyNow();
	}
	else
	{
		hide();

		gtk_window_set_decorated(GTK_WINDOW(border), has_border);
		gtk_window_set_type_hint(GTK_WINDOW(border), type);

		move(oldx, oldy);
	}
}

void gMainWindow::showActivate()
{
	bool v = isTopLevel() && isVisible() && !_no_take_focus;

	setType(GTK_WINDOW_TOPLEVEL);

	if (!_moved)
		center();
	emitOpen();
	show();
	if (v)
		present();
}

void gMainWindow::activate()
{
	if (isTopLevel() && isVisible())
		present();
}

void gMainWindow::showPopup()
{
	int x, y;
	gMouse::getScreenPos(&x, &y);
	showPopup(x, y);
}

void gMainWindow::raise()
{
	if (!isTopLevel()) { gControl::raise(); return; }
	present();
}

const char* gMainWindow::text()
{
	return _title;
}

void gMainWindow::setText(const char *txt)
{
	if (_title) g_free(_title);
	_title = g_strdup(txt);

	if (isTopLevel())
		gtk_window_set_title(GTK_WINDOW(border), txt);
}

bool gMainWindow::hasBorder()
{
	if (isTopLevel())
		return gtk_window_get_decorated(GTK_WINDOW(border));
	else
		return false;
}

bool gMainWindow::isResizable()
{
	if (isTopLevel())
		return _resizable;
	else
		return false;
}

void gMainWindow::setBorder(bool b)
{
	if (!isTopLevel())
		return;

	gtk_window_set_decorated(GTK_WINDOW(border), b);
	/*#ifdef GDK_WINDOWING_X11
	XSetWindowAttributes attr;
	attr.override_redirect = !b;
	XChangeWindowAttributes(GDK_WINDOW_XDISPLAY(border->window), GDK_WINDOW_XID(border->window), CWOverrideRedirect, &attr);
	#endif*/
}

void gMainWindow::setResizable(bool b)
{
	if (!isTopLevel())
		return;

	if (b == isResizable())
		return;

	_resizable = b;
	setGeometryHints();
}


void gMainWindow::setSkipTaskBar(bool b)
{
	if (!isTopLevel()) return;
	_skip_taskbar = b;
	gtk_window_set_skip_taskbar_hint(GTK_WINDOW(border), b);
}


/*gPicture* gMainWindow::icon()
{
	GdkPixbuf *buf;
	gPicture *pic;

	if (!isTopLevel()) return NULL;

	buf=gtk_window_get_icon(GTK_WINDOW(border));
	if (!buf) return NULL;

	pic=gPicture::fromPixbuf(buf);

	return pic;
}*/

void gMainWindow::setIcon(gPicture *pic)
{
  gPicture::assign(&_icon, pic);

	if (!isTopLevel()) return;
  gtk_window_set_icon(GTK_WINDOW(border), pic ? pic->getPixbuf() : NULL);
}


void gMainWindow::setTopOnly(bool vl)
{
	if (!isTopLevel()) return;

	_top_only = vl;
	gtk_window_set_keep_above (GTK_WINDOW(border), vl);
}


void gMainWindow::setMask(bool vl)
{
	if (_mask == vl)
		return;

	_mask = vl;
	drawMask();
}

void gMainWindow::setPicture(gPicture *pic)
{
  gPicture::assign(&_picture, pic);
  drawMask();
}

void gMainWindow::remap()
{
	if (!isVisible())
		return;

	gtk_widget_unmap(border);
	gtk_widget_map(border);

	if (_skip_taskbar) { setSkipTaskBar(false);	setSkipTaskBar(true); }
	if (_top_only) { setTopOnly(false); setTopOnly(true); }
	if (_sticky) { setSticky(false); setSticky(true); }
	if (stack) { setStacking(0); setStacking(stack); }
	X11_set_window_type(handle(), _type);
}

void gMainWindow::drawMask()
{
	bool do_remap = false;

	if (!isVisible())
		return;

#ifdef GTK3

	cairo_region_t *mask;

	if (_mask && _picture)
		mask = gdk_cairo_region_create_from_surface(_picture->getSurface());
	else
		mask = NULL;

	gdk_window_shape_combine_region(gtk_widget_get_window(border), mask, 0, 0);
	if (mask)
		cairo_region_destroy(mask);

	refresh();

#else

	GdkBitmap *mask = (_mask && _picture) ? _picture->getMask() : NULL;
	do_remap = !mask && _masked;

	gdk_window_shape_combine_mask(border->window, mask, 0, 0);

#endif

	if (_picture)
	{
		gtk_widget_set_app_paintable(border, TRUE);
		gtk_widget_realize(border);
		gtk_widget_realize(widget);
		for (int i = 0; i < controlCount(); i++)
			getControl(i)->refresh();
	}
	else if (!_transparent)
	{
		gtk_widget_set_app_paintable(border, FALSE);
		setRealBackground(background());
	}

	_masked = mask != NULL;

	if (do_remap)
		remap();
	else
	{
		if (!_skip_taskbar)
		{
			setSkipTaskBar(true);
			setSkipTaskBar(false);
		}
	}
}

int gMainWindow::menuCount()
{
	if (!menuBar) return 0;
	return gMenu::winChildCount(this);
}

void gMainWindow::setPersistent(bool vl)
{
  _persistent = vl;
}

bool gMainWindow::doClose()
{
	if (_closing || _closed)
		return false;

	if (!isTopLevel())
	{
		if (_opened)
		{
			_closing = true;
			_closed = !onClose(this);
			_closing = false;
			_opened = !_closed;
		}
		else
			_closed = true;

		if (_closed)
		{
			if (_persistent)
				hide();
			else
				destroy();
		}
	}
	else
	{
		if (_opened)
		{
			if (isModal() && !gApplication::hasLoop(this))
				return true;

			_closing = true;
			_closed = !onClose(this);
			_closing = false;
			_opened = !_closed;

			if (!_opened && isModal())
				gApplication::exitLoop(this);
		}

		if (!_opened) // && !modal())
		{
			if (_active == this)
				setActiveWindow(NULL);

			if (!isModal())
			{
				if (_persistent)
					hide();
				else
					destroy();
			}
		}
	}
	
	return _opened;
}


bool gMainWindow::close()
{
	return doClose();
}

static void hide_hidden_children(gContainer *cont)
{
	int i;
	gControl *child;

	for (i = 0;; i++)
	{
		child = cont->child(i);
		if (!child)
			break;
		if (!child->isVisible())
			gtk_widget_hide(child->border);
		else if (child->isContainer())
			hide_hidden_children((gContainer *)child);
	}
}

void gMainWindow::reparent(gContainer *newpr, int x, int y)
{
	GtkWidget *new_border;
	int w, h;
	gColor fg, bg;

	if (_xembed)
		return;

	bg = background();
	fg = foreground();

	if (isTopLevel() && newpr)
	{
		gtk_window_remove_accel_group(GTK_WINDOW(topLevel()->border), accel);

		new_border = gtk_event_box_new();
		gt_widget_reparent(widget, new_border);
		embedMenuBar(new_border);
		_no_delete = true;
		gtk_widget_destroy(border);
		_no_delete = false;

		border = new_border;
		registerControl();
		setCanFocus(true);

		setParent(newpr);
		connectParent();
		borderSignals();
		initWindow();

		setBackground(bg);
		setForeground(fg);
		setFont(font());

		checkMenuBar();

		bufX = bufY = 0;
		move(x, y);

		gtk_widget_set_size_request(border, width(), height());

		// Hidden children are incorrectly shown. Fix that!
		hideHiddenChildren();
	}
	else if ((!isTopLevel() && !newpr)
	         || (isTopLevel() && isPopup()))
	         //|| (isTopLevel() && (isPopup() ^ (type == GTK_WINDOW_POPUP))))
	{
		gtk_window_remove_accel_group(GTK_WINDOW(topLevel()->border), accel);
		// TODO: test that
		new_border = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gt_widget_reparent(widget, new_border);
		embedMenuBar(new_border);
		_no_delete = true;
		gtk_widget_destroy(border);
		_no_delete = false;

		border = new_border;
		registerControl();
		setCanFocus(true);

		if (parent())
		{
			parent()->remove(this);
			parent()->arrange();
			setParent(NULL);
		}
		initWindow();
		borderSignals();
		setBackground(bg);
		setForeground(fg);
		setFont(font());

		move(x, y);
		w = width();
		h = height();
		bufW = bufH = -1;
		gtk_widget_set_size_request(border, 1, 1);
		resize(w, h);

		hideHiddenChildren();
		_popup = false; //type == GTK_WINDOW_POPUP;
	}
	else
	{
		gContainer::reparent(newpr, x, y);
	}
}

void gMainWindow::setType(GtkWindowType type)
{
	GtkWidget *new_border;
	int w, h;
	gColor bg, fg;

	if (!isTopLevel())
		return;
	if (gtk_window_get_window_type(GTK_WINDOW(border)) == type)
		return;
		
	bg = background();
	fg = foreground();

	gtk_window_remove_accel_group(GTK_WINDOW(border), accel);
	// TODO: test that
	new_border = gtk_window_new(type);
	gt_widget_reparent(widget, new_border);
	embedMenuBar(new_border);
	_no_delete = true;
	gtk_widget_destroy(border);
	_no_delete = false;

	border = new_border;
	registerControl();
	setCanFocus(true);

	initWindow();
	borderSignals();
	setBackground(bg);
	setForeground(fg);
	setFont(font());

	w = width();
	h = height();
	bufW = bufH = -1;
	gtk_widget_set_size_request(border, 1, 1);
	resize(w, h);

	hideHiddenChildren();
}


int gMainWindow::controlCount()
{
	GList *list = gControl::controlList();
	gControl *ctrl;
	int n = 0;

	while (list)
	{
		ctrl = (gControl *)list->data;
		if (ctrl->window() == this && !ctrl->isDestroyed())
			n++;
		list = g_list_next(list);
	}

	return n;
}

gControl *gMainWindow::getControl(char *name)
{
	GList *list = gControl::controlList();
	gControl *ctrl;

	while (list)
	{
		ctrl = (gControl *)list->data;
		if (ctrl->window() == this && !strcasecmp(ctrl->name(), name) && !ctrl->isDestroyed())
			return ctrl;
		list = g_list_next(list);
	}

	return NULL;
}

gControl *gMainWindow::getControl(int index)
{
	GList *list = gControl::controlList();
	gControl *ctrl;
	int i = 0;

	while (list)
	{
		ctrl = (gControl *)list->data;
		if (ctrl->window() == this && !ctrl->isDestroyed())
		{
			if (i == index)
				return ctrl;
			i++;
		}
		list = g_list_next(list);
	}

	return NULL;
}


int gMainWindow::clientX()
{
	return 0;
}

int gMainWindow::containerX()
{
	return 0;
}

int gMainWindow::clientY()
{
	if (isMenuBarVisible())
		return menuBarHeight();
	else
		return 0;
}

int gMainWindow::containerY()
{
	return 0;
}


int gMainWindow::clientWidth()
{
	return width();
}


int gMainWindow::menuBarHeight()
{
	int h = 0;

	if (menuBar)
	{
		//gtk_widget_show(GTK_WIDGET(menuBar));
		//fprintf(stderr, "menuBarHeight: gtk_widget_get_visible: %d\n", gtk_widget_get_visible(GTK_WIDGET(menuBar)));
#ifdef GTK3
		gtk_widget_get_preferred_height(GTK_WIDGET(menuBar), NULL, &h);
#else
		GtkRequisition req = { 0, 0 };
		gtk_widget_size_request(GTK_WIDGET(menuBar), &req);
		h = req.height;
#endif
		//fprintf(stderr, "menuBarHeight: %d\n", h);
	}

	return h;
}

int gMainWindow::clientHeight()
{
	if (isMenuBarVisible())
		return height() - menuBarHeight();
	else
		return height();
}

void gMainWindow::setActiveWindow(gControl *control)
{
	gMainWindow *window = control ? control->window() : NULL;
	gMainWindow *old = _active;

	if (window == _active)
		return;

	_active = window;

	//fprintf(stderr, "setActiveWindow: %p %s\n", _active, _active ? _active->name() : "");

	if (old)
		old->emit(SIGNAL(old->onDeactivate));

	if (window)
		window->emit(SIGNAL(window->onActivate));
}

#ifdef GDK_WINDOWING_X11
bool gMainWindow::isUtility() const
{
	return _utility;
}

void gMainWindow::setUtility(bool v)
{
	bool remap = false;

	if (!isTopLevel())
		return;

	// TODO: works only if the window is not mapped!

	_utility = v;
#if GTK_CHECK_VERSION(2, 20, 0)
	if (gtk_widget_get_mapped(border))
#else
    if (GTK_WIDGET_MAPPED(border))
#endif
	{
		remap = true;
		gtk_widget_unmap(border);
	}

	gtk_window_set_type_hint(GTK_WINDOW(border), v ? GDK_WINDOW_TYPE_HINT_DIALOG : GDK_WINDOW_TYPE_HINT_NORMAL);

	if (remap)
		gtk_widget_map(border);
}
#else
bool gMainWindow::isUtility()
{
	return _utility;
}

void gMainWindow::setUtility(bool v)
{
	_utility = v;
}
#endif

void gMainWindow::configure()
{
	static bool init = FALSE;
	static GB_FUNCTION _init_menubar_shortcut_func;

	int h;

	if (bufW < 1 || bufH < 1)
		return;

	if (_initMenuBar != isMenuBarVisible())
	{
		_initMenuBar = !_initMenuBar;

		if (!init)
		{
			GB.GetFunction(&_init_menubar_shortcut_func, (void *)GB.FindClass("_Gui"), "_InitMenuBarShortcut", NULL, NULL);
			init = TRUE;
		}

		GB.Push(1, GB_T_OBJECT, hFree);
		GB.Call(&_init_menubar_shortcut_func, 1, FALSE);
	}

	h = menuBarHeight();

	#ifdef DEBUG_RESIZE
	fprintf(stderr, "configure: %s: %d %d - %d %d\n", name(), isMenuBarVisible(), h, width(), height());
	#endif

	if (isMenuBarVisible())
	{
		gtk_fixed_move(layout, GTK_WIDGET(menuBar), 0, 0);
		if (h > 1)
			gtk_widget_set_size_request(GTK_WIDGET(menuBar), width(), h);
		gtk_fixed_move(layout, widget, 0, h);
		gtk_widget_set_size_request(widget, width(), Max(0, height() - h));
	}
	else
	{
		if (layout)
		{
			if (menuBar)
				gtk_fixed_move(layout, GTK_WIDGET(menuBar), 0, -h);
			gtk_fixed_move(layout, widget, 0, 0);
		}
		gtk_widget_set_size_request(widget, width(), height());
	}
}

bool gMainWindow::setMenuBarVisible(bool v)
{
	if (_showMenuBar == v)
		return TRUE;

	_showMenuBar = v;

	if (!menuBar)
		return TRUE;

	configure();
	#ifdef DEBUG_RESIZE
	fprintf(stderr, "#5\n");
	#endif
	performArrange();

	return FALSE;
}

bool gMainWindow::isMenuBarVisible()
{
	//fprintf(stderr, "isMenuBarVisible: %d\n", !!(menuBar && !_hideMenuBar && _showMenuBar));
	return menuBar && !_hideMenuBar && _showMenuBar; //|| (menuBar && GTK_WIDGET_MAPPED(GTK_WIDGET(menuBar)));
}

void gMainWindow::updateFont()
{
	gContainer::updateFont();
	gMenu::updateFont(this);
	emit(SIGNAL(onFontChange));
}

void gMainWindow::checkMenuBar()
{
	int i;
	gMenu *menu;

	//fprintf(stderr, "gMainWindow::checkMenuBar\n");

	if (menuBar)
	{
		_hideMenuBar = true;
		for (i = 0;; i++)
		{
			menu = gMenu::winChildMenu(this, i);
			if (!menu)
				break;
			if (menu->isVisible() && !menu->isSeparator())
			{
				_hideMenuBar = false;
				break;
			}
		}
	}

	configure();
	#ifdef DEBUG_RESIZE
	fprintf(stderr, "#6\n");
	#endif
	performArrange();
}

void gMainWindow::embedMenuBar(GtkWidget *border)
{
	if (menuBar)
	{
		// layout is automatically destroyed ?
		layout = GTK_FIXED(gtk_fixed_new());
		
#ifdef GTK3
		g_signal_connect_after(G_OBJECT(layout), "size-allocate", G_CALLBACK(cb_resize_layout), (gpointer)this);
#endif

		g_object_ref(G_OBJECT(menuBar));

		if (gtk_widget_get_parent(GTK_WIDGET(menuBar)))
			gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(GTK_WIDGET(menuBar))), GTK_WIDGET(menuBar));

		gtk_fixed_put(layout, GTK_WIDGET(menuBar), 0, 0);

		g_object_unref(G_OBJECT(menuBar));

		gt_widget_reparent(widget, GTK_WIDGET(layout));
		gtk_container_add(GTK_CONTAINER(border), GTK_WIDGET(layout));

		gtk_widget_show(GTK_WIDGET(menuBar));
		gtk_widget_show(GTK_WIDGET(layout));
		gtk_widget_show(GTK_WIDGET(widget));

		gMenu::updateFont(this);
		gMenu::updateColor(this);

		checkMenuBar();
	}
}

/*bool gMainWindow::getScreenPos(int *x, int *y)
{
	return gContainer::getScreenPos(x, y);
}*/

double gMainWindow::opacity()
{
	if (isTopLevel())
#if GTK_CHECK_VERSION(3, 8, 0)
		return gtk_widget_get_opacity(border);
#else
		return gtk_window_get_opacity(GTK_WINDOW(border));
#endif
	else
		return 1.0;
}

void gMainWindow::setOpacity(double v)
{
	if (isTopLevel())
#if GTK_CHECK_VERSION(3, 8, 0)
		gtk_widget_set_opacity(border, v);
#else
		gtk_window_set_opacity(GTK_WINDOW(border), v);
#endif
}

int gMainWindow::screen()
{
	gMainWindow *tl = topLevel();
	return gdk_screen_get_number(gtk_window_get_screen(GTK_WINDOW(tl->border)));
}

void gMainWindow::emitResize()
{
	if (bufW == _resize_last_w && bufH == _resize_last_h)
		return;

	#ifdef DEBUG_RESIZE
	fprintf(stderr, "emitResize: %s: %d %d\n", name(), bufW, bufH);
	#endif
	_resize_last_w = bufW;
	_resize_last_h = bufH;
	configure();
	#ifdef DEBUG_RESIZE
	fprintf(stderr, "#7\n");
	#endif
	performArrange();
	emit(SIGNAL(onResize));
}

static void emit_resize_later(gMainWindow *window)
{
	window->emitResize();
}

void gMainWindow::emitResizeLater()
{
	GB.Post((GB_CALLBACK)emit_resize_later, (intptr_t)this);
}

void gMainWindow::setGeometryHints()
{
	GdkGeometry geometry;

	if (isTopLevel())
	{
		if (isResizable())
		{
			if (isModal())
			{
				geometry.min_width = _min_w;
				geometry.min_height = _min_h;
			}
			else
			{
				geometry.min_width = 0;
				geometry.min_height = 0;
			}

			geometry.max_width = 32767;
			geometry.max_height = 32767;
		}
		else
		{
			geometry.min_width = width();
			geometry.min_height = height();
			geometry.max_width = width();
			geometry.max_height = height();
		}

		gtk_window_set_geometry_hints(GTK_WINDOW(border), NULL, &geometry, (GdkWindowHints)(GDK_HINT_MIN_SIZE | GDK_HINT_MAX_SIZE | GDK_HINT_POS));
		//gdk_window_set_geometry_hints(gtk_widget_get_window(border), &geometry, (GdkWindowHints)(GDK_HINT_MIN_SIZE | GDK_HINT_POS));
	}
}

void gMainWindow::setBackground(gColor vl)
{
	_bg = vl;
	if (!_transparent)
		gControl::setBackground(vl);
}

void gMainWindow::setTransparent(bool vl)
{
	if (!vl)
		return;

	_transparent = TRUE;

	if (!isVisible())
		return;

#ifdef GTK3
	GdkScreen *screen = NULL;
	GdkVisual *visual = NULL;

	screen = gtk_widget_get_screen(border);
	visual = gdk_screen_get_rgba_visual(screen);
	if (visual == NULL)
		return;
#else
	GdkScreen *screen;
	GdkColormap *colormap;

	screen = gtk_widget_get_screen(border);
	colormap = gdk_screen_get_rgba_colormap(screen);
	if (colormap == NULL)
		return;
#endif

	gtk_widget_unrealize(border);

	gtk_widget_set_app_paintable(border, TRUE);

#ifdef GTK3
	gtk_widget_set_visual(border, visual);
#else
	gtk_widget_set_colormap(border, colormap);
#endif

	gtk_widget_realize(border);

	int w = width();
	int h = height();

	bufW = w - 1;
	resize(w, h);

	//gtk_window_present(GTK_WINDOW(border));
}

bool gMainWindow::closeAll()
{
	int i;
	gMainWindow *win;

	for(i = 0; i < count(); i++)
	{
		win = get(i);
		if (!win)
			break;
		if (!win->isTopLevel())
			continue;
		if (win == gApplication::mainWindow())
			continue;
		if (win->close())
			return true;
	}

	return false;
}

void gMainWindow::setNoTakeFocus(bool v)
{
	_no_take_focus = v;
	if (isTopLevel())
		gtk_window_set_focus_on_map(GTK_WINDOW(border), !_no_take_focus);
}

void gMainWindow::calcCsdSize()
{
	GtkAllocation ba;
	GtkAllocation wa;
	
	if (_csd_w >= 0)
		return;
		
	if (!isTopLevel())
	{
		_csd_w = _csd_h = 0;
		return;
	}
		
	gtk_widget_get_allocation(border, &ba);
	gtk_widget_get_allocation(layout ? GTK_WIDGET(layout) : widget, &wa);
	
	if (wa.width == 1 && wa.height == 1)
		return;

	_csd_w = ba.width - wa.width;
	_csd_h = ba.height - wa.height;
	if (!layout)
		_csd_h -= clientY();
	#ifdef DEBUG_RESIZE
	fprintf(stderr, "calcCsdSize: %s: csd = %d %d\n", name(), _csd_w, _csd_h);
	#endif
}
