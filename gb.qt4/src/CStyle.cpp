/***************************************************************************

  CStyle.cpp

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

#define __CStyle_CPP

#include <QApplication>
#include <QToolTip>

#include "gambas.h"
#include "main.h"
#include "gb.draw.h"
#include "cpaint_impl.h"
#include "CPicture.h"
#include "CWidget.h"
#include "CWindow.h"
#include "CFont.h"
#include "CScreen.h"

bool CSTYLE_fix_breeze = false;
bool CSTYLE_fix_oxygen = false;

static char *_style_name = NULL;

static QWidget *_fake = 0;

static bool _is_breeze = false;
static bool _is_oxygen = false;
static bool _is_windows = false;
static bool _is_gtk = false;
static bool _is_qtcurve = false;
static bool _is_plastique = false;

static QWidget *get_fake_widget()
{
	if (!_fake)
		_fake = new QWidget;
	return _fake;
}

static char *get_style_name()
{
	if (!_style_name)
	{
		if (CSTYLE_fix_breeze)
		{
			_style_name = GB.NewZeroString("breeze");
		}
		else if (CSTYLE_fix_oxygen)
		{
			_style_name = GB.NewZeroString("oxygen");
		}
		else
		{
			const char *name = qApp->style()->metaObject()->className();
			int len = strlen(name);
		
			if (len >= 6 && strncasecmp(&name[len - 5], "style", 5) == 0)
				len -= 5;
			if (len >= 3 && strncmp(&name[len - 2], "::", 2) == 0)
				len -= 2;
			if (name[0] == 'Q' && isupper(name[1]))
			{
				len--;
				name++;
			}
			
			_style_name = GB.NewString(NULL, len);
			for (int i = 0; i < len; i++)
				_style_name[i] = tolower(name[i]);
		}
		
		_is_breeze = ::strcmp(_style_name, "breeze") == 0;
		_is_oxygen = ::strcmp(_style_name, "oxygen") == 0;
		_is_windows = ::strcmp(_style_name, "windows") == 0;
		_is_gtk = ::strcmp(_style_name, "gtk") == 0;
		_is_qtcurve = ::strcmp(_style_name, "qtcurve") == 0;
		_is_plastique = ::strcmp(_style_name, "plastique") == 0;
	}
	
	return _style_name;
}

static void init_option(QStyleOption &opt, int x, int y, int w, int h, int state, GB_COLOR color = COLOR_DEFAULT, QPalette::ColorRole role = QPalette::Window)
{
	opt.rect = QRect(x, y, w ,h);
	opt.state = QStyle::State_None;

	if (!(state & GB_DRAW_STATE_DISABLED))
		opt.state |= QStyle::State_Enabled;
	if (state & GB_DRAW_STATE_FOCUS)
		opt.state |= QStyle::State_HasFocus;
	if (state & GB_DRAW_STATE_HOVER)
		opt.state |= QStyle::State_MouseOver;
	if (state & GB_DRAW_STATE_ACTIVE)
		opt.state |= QStyle::State_On | QStyle::State_Sunken | QStyle::State_Active;

	if (color != GB_COLOR_DEFAULT)
	{
		QPalette palette;
		palette.setColor(role, TO_QCOLOR(color));
		opt.palette = palette;
	}
	
	if (state & GB_DRAW_STATE_DISABLED)
		opt.palette.setCurrentColorGroup(QPalette::Disabled);
}

static void paint_focus(QPainter *p, int x, int y, int w, int h, int state)
{
	//bool do_clip = FALSE;
	QStyleOptionFocusRect opt;
	
	if ((state & GB_DRAW_STATE_DISABLED) || !(state & GB_DRAW_STATE_FOCUS))
		return;
	
	init_option(opt, x, y, w, h, state);
	
	/*if (::strcmp(qApp->style()->metaObject()->className(), "QtCurve::Style") == 0)
	{
		QPainterPath clip;

		p->save();

		clip.addRect(x, y, w, 1);
		clip.addRect(x, y, 1, h);
		clip.addRect(x, y + h - 1, w, 1);
		clip.addRect(x + w - 1, y, 1, h);
		p->setClipPath(clip);
		do_clip = TRUE;
	}*/

	p->save();
	p->setBrush(QBrush());

	QApplication::style()->drawPrimitive(QStyle::PE_FrameFocusRect, &opt, p);

	p->restore();

	/*if (do_clip)
		p->restore();*/
}

static void style_arrow(QPainter *p, int x, int y, int w, int h, int type, int state)
{
	QStyleOption opt;
	QStyle::PrimitiveElement pe;
	
	init_option(opt, x, y, w, h, state);
	
	switch (type)
	{
		case ALIGN_NORMAL: pe = GB.System.IsRightToLeft() ? QStyle::PE_IndicatorArrowLeft : QStyle::PE_IndicatorArrowRight; break;
		case ALIGN_LEFT: pe = QStyle::PE_IndicatorArrowLeft; break;
		case ALIGN_RIGHT: pe = QStyle::PE_IndicatorArrowRight; break;
		case ALIGN_TOP: pe = QStyle::PE_IndicatorArrowUp; break;
		case ALIGN_BOTTOM: pe = QStyle::PE_IndicatorArrowDown; break;
		default:
			return;
	}

	QApplication::style()->drawPrimitive(pe, &opt, p);
}

static void style_check(QPainter *p, int x, int y, int w, int h, int value, int state)
{
	QStyleOptionButton opt;
	int d;
	
	get_style_name();
	if (_is_oxygen || _is_breeze)
		d = 2;
	else
		d = 0;
	
	x -= d;
	y -= d;
	w += d * 2;
	h += d * 2;
	
	init_option(opt, x, y, w, h, state);
	
	if (value == 1)
		opt.state |= QStyle::State_NoChange;
	else if (value)
		opt.state |= QStyle::State_On;
	else
		opt.state |= QStyle::State_Off;
	
	QApplication::style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &opt, p);
	paint_focus(p, x, y, w, h, state);
}

static void style_option(QPainter *p, int x, int y, int w, int h, int value, int state)
{
	QStyleOptionButton opt;
	int d;

	get_style_name();
	if (_is_oxygen || _is_breeze)
		d = 2;
	else
		d = 0;
	
	x -= d;
	y -= d;
	w += d * 2;
	h += d * 2;
	
	init_option(opt, x, y, w, h, state);
	
	if (value)
		opt.state |= QStyle::State_On;
	
	QApplication::style()->drawPrimitive(QStyle::PE_IndicatorRadioButton, &opt, p);
	paint_focus(p, x, y, w, h, state);
}

static void style_separator(QPainter *p, int x, int y, int w, int h, int vertical, int state)
{
	QStyleOption opt;
	init_option(opt, x, y, w, h, state);
	
	if (vertical)
		opt.state |= QStyle::State_Horizontal;
	
	QApplication::style()->drawPrimitive(QStyle::PE_IndicatorToolBarSeparator, &opt, p);
}


static void style_button(QPainter *p, int x, int y, int w, int h, int value, int state, int flat)
{
	if (flat)
	{
		QStyleOptionToolButton opt;
		
		init_option(opt, x, y, w, h, state);
		
		//opt.state |= QStyle::State_Raised;
		
		if (value)
			opt.state |= QStyle::State_On;

		//opt.state &= ~QStyle::State_HasFocus;
		opt.state |= QStyle::State_AutoRaise;
		if (opt.state & QStyle::State_MouseOver)
			opt.state |= QStyle::State_Raised;
		
		if (opt.state & (QStyle::State_Sunken | QStyle::State_On | QStyle::State_MouseOver))
		{
			QApplication::style()->drawPrimitive(QStyle::PE_PanelButtonTool, &opt, p);
		}
	}
	else
	{
		QStyleOptionButton opt;
	
		init_option(opt, x, y, w, h, state);
		
		opt.state |= QStyle::State_Raised;
		
		if (value)
			opt.state |= QStyle::State_On;
	
		QApplication::style()->drawPrimitive(QStyle::PE_PanelButtonCommand, &opt, p);
	}
	
	paint_focus(p, x, y, w, h, state);
}
			
static void style_panel(QPainter *p, int x, int y, int w, int h, int border, int state)
{
	QStyleOptionFrame opt;
	init_option(opt, x, y, w, h, state);
	
	CCONTAINER_draw_border_without_widget(p, border, opt);
}
			
static void style_handle(QPainter *p, int x, int y, int w, int h, int vertical, int state)
{
	QStyleOption opt;
	init_option(opt, x, y, w, h, state);
	
	if (!vertical)
		opt.state |= QStyle::State_Horizontal;
	
	QApplication::style()->drawPrimitive(QStyle::PE_IndicatorDockWidgetResizeHandle, &opt, p);
	paint_focus(p, x, y, w, h, state);
}


static void style_box(QPainter *p, int x, int y, int w, int h, int state, GB_COLOR color)
{
	QStyleOptionFrame opt;
	
	//if (GB.Is(d->device, CLASS_DrawingArea))
	//	opt.begin(QWIDGET(d->device));
	
	init_option(opt, x, y, w, h, state, color, QPalette::Base);

	opt.lineWidth = QApplication::style()->pixelMetric(QStyle::PM_DefaultFrameWidth, &opt);
	opt.midLineWidth = 0;
	opt.state |= QStyle::State_Sunken;
	p->save();
	p->setBrush(Qt::NoBrush);
	//opt.features = QStyleOptionFrameV2::None;
	
	if (color == GB_COLOR_DEFAULT)
		QApplication::style()->drawPrimitive(QStyle::PE_FrameLineEdit, &opt, p);
	else
	{
		get_style_name();
		if (_is_gtk)
		{
			QWidget *w = get_fake_widget();
			w->setAttribute(Qt::WA_SetPalette, true);
			QApplication::style()->drawPrimitive(QStyle::PE_PanelLineEdit, &opt, p, w);
			w->setAttribute(Qt::WA_SetPalette, false);
		}
		else
			QApplication::style()->drawPrimitive(QStyle::PE_PanelLineEdit, &opt, p);
	}

	p->restore();
	//paint_focus(d, x, y, w, h, state);
	//if (state & GB_DRAW_STATE_FOCUS)
	//	QApplication::style()->drawControl(QStyle::CE_FocusFrame, &opt, DP(d), GB.Is(d->device, CLASS_DrawingArea) ? QWIDGET(d->device) : NULL);
}

BEGIN_PROPERTY(Style_ScrollbarSize)

	GB.ReturnInteger(qApp->style()->pixelMetric(QStyle::PM_ScrollBarExtent));

END_PROPERTY

BEGIN_PROPERTY(Style_ScrollbarSpacing)

	get_style_name();
	if (_is_breeze)
		GB.ReturnInteger(0);
	else
		GB.ReturnInteger(qMax(0, qApp->style()->pixelMetric(QStyle::PM_ScrollView_ScrollBarSpacing)));

END_PROPERTY

BEGIN_PROPERTY(Style_FrameWidth)

	get_style_name();
	if (_is_breeze)
		GB.ReturnInteger(2);
	else
		GB.ReturnInteger(qApp->style()->pixelMetric(QStyle::QStyle::PM_ComboBoxFrameWidth));

END_PROPERTY

BEGIN_PROPERTY(Style_BoxFrameWidth)

	int w = qApp->style()->pixelMetric(QStyle::QStyle::PM_ComboBoxFrameWidth);
	GB.ReturnInteger(w);

END_PROPERTY

BEGIN_PROPERTY(Style_BoxFrameHeight)

	int w = qApp->style()->pixelMetric(QStyle::QStyle::PM_ComboBoxFrameWidth);
	GB.ReturnInteger(w);

END_PROPERTY

BEGIN_PROPERTY(Style_Name)

	GB.ReturnString(get_style_name());

END_PROPERTY

#define GET_COORD() \
	int x, y, w, h; \
\
	QPainter *p = PAINT_get_current(); \
	if (!p) \
		return; \
\
	x = VARG(x); \
	y = VARG(y); \
	w = VARG(w); \
	h = VARG(h); \
\
	if (w < 1 || h < 1) \
		return;

BEGIN_METHOD(Style_PaintArrow, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_INTEGER type; GB_INTEGER state)

	GET_COORD();
	style_arrow(p, x, y, w, h, VARG(type), VARGOPT(state, GB_DRAW_STATE_NORMAL));

END_METHOD

BEGIN_METHOD(Style_PaintCheck, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_INTEGER value; GB_INTEGER state)

	GET_COORD();
	style_check(p, x, y, w, h, VARG(value), VARGOPT(state, GB_DRAW_STATE_NORMAL));

END_METHOD

BEGIN_METHOD(Style_PaintOption, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_BOOLEAN value; GB_INTEGER state)

	GET_COORD();
	style_option(p, x, y, w, h, VARG(value), VARGOPT(state, GB_DRAW_STATE_NORMAL));

END_METHOD

BEGIN_METHOD(Style_PaintSeparator, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_BOOLEAN vertical; GB_INTEGER state)

	GET_COORD();
	style_separator(p, x, y, w, h, VARGOPT(vertical, FALSE), VARGOPT(state, GB_DRAW_STATE_NORMAL));

END_METHOD

BEGIN_METHOD(Style_PaintButton, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_BOOLEAN value; GB_INTEGER state; GB_BOOLEAN flat)

	GET_COORD();
	style_button(p, x, y, w, h, VARG(value), VARGOPT(state, GB_DRAW_STATE_NORMAL), VARGOPT(flat, FALSE));

END_METHOD

BEGIN_METHOD(Style_PaintPanel, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_INTEGER border; GB_INTEGER state)

	GET_COORD();
	style_panel(p, x, y, w, h, VARG(border), VARGOPT(state, GB_DRAW_STATE_NORMAL));

END_METHOD

BEGIN_METHOD(Style_PaintHandle, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_BOOLEAN vertical; GB_INTEGER state)

	GET_COORD();
	style_handle(p, x, y, w, h, VARGOPT(vertical, FALSE), VARGOPT(state, GB_DRAW_STATE_NORMAL));

END_METHOD

BEGIN_METHOD(Style_PaintBox, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_INTEGER state; GB_INTEGER color)

	GET_COORD();
	style_box(p, x, y, w, h, VARGOPT(state, GB_DRAW_STATE_NORMAL), VARGOPT(color, GB_COLOR_DEFAULT));

END_METHOD

BEGIN_METHOD(Style_StateOf, GB_OBJECT control)

	CWIDGET *control = (CWIDGET *)VARG(control);
	QWidget *widget;
	int state;
	bool design;

	if (GB.CheckObject(control))
		return;

	widget = QWIDGET(control);
	design = CWIDGET_is_design(control);

	state = GB_DRAW_STATE_NORMAL;
	if (!widget->isEnabled())
		state |= GB_DRAW_STATE_DISABLED;
	if (widget->hasFocus() && !design)
		state |= GB_DRAW_STATE_FOCUS;
	if (CWIDGET_is_visible(control) && control->flag.inside && !design)
		state |= GB_DRAW_STATE_HOVER;

	GB.ReturnInteger(state);

END_METHOD

BEGIN_METHOD(Style_BackgroundOf, GB_OBJECT control)

	CWIDGET *control = (CWIDGET *)VARG(control);

	if (GB.CheckObject(control))
		return;

	GB.ReturnInteger(CWIDGET_get_real_background(control));

END_METHOD

BEGIN_METHOD(Style_ForegroundOf, GB_OBJECT control)

	CWIDGET *control = (CWIDGET *)VARG(control);

	if (GB.CheckObject(control))
		return;

	GB.ReturnInteger(CWIDGET_get_real_foreground(control));

END_METHOD

#if 0
BEGIN_METHOD(Style_FontOf, GB_OBJECT control)

	CWIDGET *control = (CWIDGET *)VARG(control);

	if (GB.CheckObject(control))
		return;

	GB.ReturnObject(CWIDGET_get_real_font(control));

END_METHOD
#endif

BEGIN_METHOD_VOID(Style_exit)

	GB.FreeString(&_style_name);

END_METHOD

GB_DESC StyleDesc[] =
{
	GB_DECLARE("Style", 0), GB_VIRTUAL_CLASS(),
	
	GB_STATIC_METHOD("_exit", NULL, Style_exit, NULL),
	
	GB_STATIC_PROPERTY_READ("ScrollbarSize", "i", Style_ScrollbarSize),
	GB_STATIC_PROPERTY_READ("ScrollbarSpacing", "i", Style_ScrollbarSpacing),
	GB_STATIC_PROPERTY_READ("FrameWidth", "i", Style_FrameWidth),
	GB_STATIC_PROPERTY_READ("TextBoxFrameWidth", "i", Style_FrameWidth),
	GB_STATIC_PROPERTY_READ("BoxFrameWidth", "i", Style_BoxFrameWidth),
	GB_STATIC_PROPERTY_READ("BoxFrameHeight", "i", Style_BoxFrameHeight),
	GB_STATIC_PROPERTY_READ("Name", "s", Style_Name),
	
	GB_STATIC_METHOD("PaintArrow", NULL, Style_PaintArrow, "(X)i(Y)i(Width)i(Height)i(Type)i[(Flag)i]"),
	GB_STATIC_METHOD("PaintCheck", NULL, Style_PaintCheck, "(X)i(Y)i(Width)i(Height)i(Value)i[(Flag)i]"),
	GB_STATIC_METHOD("PaintOption", NULL, Style_PaintOption, "(X)i(Y)i(Width)i(Height)i(Value)b[(Flag)i]"),
	GB_STATIC_METHOD("PaintSeparator", NULL, Style_PaintSeparator, "(X)i(Y)i(Width)i(Height)i[(Vertical)b(Flag)i]"),
	GB_STATIC_METHOD("PaintButton", NULL, Style_PaintButton, "(X)i(Y)i(Width)i(Height)i(Value)b[(Flag)i(Flat)b]"),
	GB_STATIC_METHOD("PaintPanel", NULL, Style_PaintPanel, "(X)i(Y)i(Width)i(Height)i(Border)i[(Flag)i]"),
	GB_STATIC_METHOD("PaintHandle", NULL, Style_PaintHandle, "(X)i(Y)i(Width)i(Height)i[(Vertical)b(Flag)i]"),
	GB_STATIC_METHOD("PaintBox", NULL, Style_PaintBox, "(X)i(Y)i(Width)i(Height)i[(Flag)i(Color)i]"),
	
	GB_CONSTANT("Normal", "i", GB_DRAW_STATE_NORMAL),
	GB_CONSTANT("Disabled", "i", GB_DRAW_STATE_DISABLED),
	GB_CONSTANT("HasFocus", "i", GB_DRAW_STATE_FOCUS),
	GB_CONSTANT("Hovered", "i", GB_DRAW_STATE_HOVER),
	GB_CONSTANT("Active", "i", GB_DRAW_STATE_ACTIVE),

	GB_STATIC_METHOD("StateOf", "i", Style_StateOf, "(Control)Control;"),
	GB_STATIC_METHOD("BackgroundOf", "i", Style_BackgroundOf, "(Control)Control;"),
	GB_STATIC_METHOD("ForegroundOf", "i", Style_ForegroundOf, "(Control)Control;"),
	//GB_STATIC_METHOD("FontOf", "Font", Style_FontOf, "(Control)Control;"),

	GB_END_DECLARE
};

