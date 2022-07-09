/***************************************************************************

  main.cpp

  (c) Benoît Minisini <gambas@users.sourceforge.net>

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

#define __MAIN_C

#include "main.h"

#include <QAbstractNativeEventFilter>
#include <QPointer>
#include <QWindow>
#include <QApplication>

static QPointer<QWidget> _mouseGrabber = 0;
static QPointer<QWidget> _keyboardGrabber = 0;

//-------------------------------------------------------------------------

static void platform_init(void)
{
}

static void platform_exit(void)
{
}

//-------------------------------------------------------------------------

static void release_grab()
{
	_mouseGrabber = QWidget::mouseGrabber();
	_keyboardGrabber = QWidget::keyboardGrabber();

	if (_mouseGrabber)
	{
		//qDebug("releaseMouse");
		_mouseGrabber->releaseMouse();
	}
	if (_keyboardGrabber)
	{
		//qDebug("releaseKeyboard");
		_keyboardGrabber->releaseKeyboard();
	}

	/*#ifndef NO_X_WINDOW
	if (qApp->activePopupWidget())
	{
		XUngrabPointer(QX11Info::display(), CurrentTime);
		XFlush(QX11Info::display());
	}
	#endif*/
}

static void unrelease_grab()
{
	if (_mouseGrabber)
	{
		//qDebug("grabMouse");
		_mouseGrabber->grabMouse();
		_mouseGrabber = 0;
	}

	if (_keyboardGrabber)
	{
		//qDebug("grabKeyboard");
		_keyboardGrabber->grabKeyboard();
		_keyboardGrabber = 0;
	}
}

static int get_last_key_code(void)
{
	return 0;
}

//-------------------------------------------------------------------------

static int desktop_get_resolution_x(void)
{
	return 96;
}

static int desktop_get_resolution_y(void)
{
	return 96;
}

static void desktop_screenshot(QPixmap *pixmap, int x, int y, int w, int h)
{
	//*pixmap = QPixmap::grabWindow(QX11Info::appRootWindow(), x, y, w, h);
}

//-------------------------------------------------------------------------

static int window_get_virtual_desktop(QWidget *window)
{
	return 0; //X11_window_get_desktop(window->winId());
}

static void window_set_virtual_desktop(QWidget *window, bool visible, int desktop)
{
	//X11_window_set_desktop(window->winId(), window->isVisible(), desktop);
}

static void window_remap(QWidget *window)
{
	//X11_window_remap(window->effectiveWinId());
}

static void window_set_properties(QWidget *window, int which, QT_WINDOW_PROP *prop)
{
	/*X11_flush();

	if (which & (PROP_STACKING | PROP_SKIP_TASKBAR))
	{
		X11_window_change_begin(window->effectiveWinId(), window->isVisible());

		if (which & PROP_STACKING)
		{
			X11_window_change_property(X11_atom_net_wm_state_above, prop->stacking == 1);
			X11_window_change_property(X11_atom_net_wm_state_stays_on_top, prop->stacking == 1);
			X11_window_change_property(X11_atom_net_wm_state_below, prop->stacking == 2);
		}
		if (which & PROP_SKIP_TASKBAR)
			X11_window_change_property(X11_atom_net_wm_state_skip_taskbar, prop->skipTaskbar);

		X11_window_change_end();
	}

	if (which & PROP_BORDER)
		X11_set_window_decorated(window->effectiveWinId(), prop->border);

	if (which & PROP_STICKY)
		X11_window_set_desktop(window->effectiveWinId(), window->isVisible(), prop->sticky ? 0xFFFFFFFF : X11_get_current_desktop());

	X11_flush();*/
	
	Qt::WindowFlags flags = window->windowFlags();
	bool visible = window->isVisible();
	
	if (prop->stacking == 1)
		flags |= Qt::WindowStaysOnTopHint;
	else
		flags &= ~Qt::WindowStaysOnTopHint;
	
	if (prop->stacking == 2)
		flags |= Qt::WindowStaysOnBottomHint;
	else
		flags &= ~Qt::WindowStaysOnBottomHint;
	
	if (!prop->border)
		flags |= Qt::FramelessWindowHint;
	else
		flags &= ~Qt::FramelessWindowHint;
	
	window->setWindowFlags(flags);
	if (visible)
		window->show();
}

static void window_set_user_time(QWidget *window, int timestamp)
{
	//X11_window_set_user_time(window->effectiveWinId(), 0);
}

static void window_set_transient_for(QWidget *window, QWidget *parent)
{
#if QT_VERSION >= 0x051300
	window->windowHandle()->setTransientParent(parent->windowHandle());
#endif
}

//-------------------------------------------------------------------------

extern "C" {

const GB_INTERFACE *GB_PTR EXPORT;

void *GB_QT5_WAYLAND_1[] EXPORT = {

  (void *)QT_PLATFORM_INTERFACE_VERSION,
  
  (void *)platform_init,
  (void *)platform_exit,
  
  (void *)release_grab,
  (void *)unrelease_grab,
  (void *)get_last_key_code,
  
  (void *)desktop_get_resolution_x,
  (void *)desktop_get_resolution_y,
  (void *)desktop_screenshot,
  
  (void *)window_get_virtual_desktop,
  (void *)window_set_virtual_desktop,
  (void *)window_remap,
  (void *)window_set_properties,
  (void *)window_set_user_time,
  (void *)window_set_transient_for,
  
  NULL
  };


int EXPORT GB_INFO(const char *key, void **value)
{
	/*if (!strcasecmp(key, "DISPLAY"))
	{
		*value = (void *)QX11Info::display();
		return TRUE;
	}
	else if (!strcasecmp(key, "ROOT_WINDOW"))
	{
		*value = (void *)QX11Info::appRootWindow();
		return TRUE;
	}
	else if (!strcasecmp(key, "TIME"))
	{
		*value = (void *)QX11Info::appTime();
		return TRUE;
	}
	else*/
		return FALSE;
}

int EXPORT GB_INIT(void)
{
	return 0;
}

void EXPORT GB_EXIT()
{
}

}

