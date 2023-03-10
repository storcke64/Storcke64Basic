/***************************************************************************

  CPanel.cpp

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>
  (c) Benoît Minisini <benoit.minisini@gambas-basic.org>

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

#define __CPANEL_CPP

#include "CPanel.h"


BEGIN_METHOD(CPANEL_new, GB_OBJECT parent)

	InitControl(new gPanel(CONTAINER(VARG(parent))), (CWIDGET*)THIS);

END_METHOD


BEGIN_METHOD(CHBOX_new, GB_OBJECT parent)
	
	InitControl(new gPanel(CONTAINER(VARG(parent))), (CWIDGET*)THIS);
	PANEL->setArrange(ARRANGE_HORIZONTAL);
	//WIDGET->setAutoSize(true);

END_METHOD


BEGIN_METHOD(CVBOX_new, GB_OBJECT parent)

	InitControl(new gPanel(CONTAINER(VARG(parent))), (CWIDGET*)THIS);
	PANEL->setArrange(ARRANGE_VERTICAL);
	//WIDGET->setAutoSize(true);

END_METHOD


BEGIN_METHOD(CHPANEL_new, GB_OBJECT parent)

	InitControl(new gPanel(CONTAINER(VARG(parent))), (CWIDGET*)THIS);
	PANEL->setArrange(ARRANGE_LEFT_RIGHT);
	//WIDGET->setAutoSize(true);

END_METHOD


BEGIN_METHOD(CVPANEL_new, GB_OBJECT parent)

	InitControl(new gPanel(CONTAINER(VARG(parent))), (CWIDGET*)THIS);
	PANEL->setArrange(ARRANGE_TOP_BOTTOM);
	//WIDGET->setAutoSize(true);

END_METHOD

BEGIN_PROPERTY(CPANEL_border)

	if (READ_PROPERTY) { GB.ReturnInteger(PANEL->getBorder()); return; }
	PANEL->setBorder(VPROP(GB_INTEGER));

END_PROPERTY

//-------------------------------------------------------------------------

GB_DESC CPanelDesc[] =
{
  GB_DECLARE("Panel", sizeof(CPANEL)), GB_INHERITS("Container"),

  GB_METHOD("_new", 0, CPANEL_new, "(Parent)Container;"),

  GB_PROPERTY("Border", "i", CPANEL_border),
  
  ARRANGEMENT_PROPERTIES,

  PANEL_DESCRIPTION,
	
  GB_END_DECLARE
};


GB_DESC CHBoxDesc[] =
{
  GB_DECLARE("HBox", sizeof(CPANEL)), GB_INHERITS("Container"),

  GB_METHOD("_new", 0, CHBOX_new, "(Parent)Container;"),

	ARRANGEMENT_FLAG_PROPERTIES,

  HBOX_DESCRIPTION,

  GB_END_DECLARE
};


GB_DESC CVBoxDesc[] =
{
  GB_DECLARE("VBox", sizeof(CPANEL)), GB_INHERITS("Container"),

  GB_METHOD("_new", 0, CVBOX_new, "(Parent)Container;"),

	ARRANGEMENT_FLAG_PROPERTIES,

  VBOX_DESCRIPTION,

  GB_END_DECLARE
};


GB_DESC CHPanelDesc[] =
{
  GB_DECLARE("HPanel", sizeof(CPANEL)), GB_INHERITS("Container"),

  GB_METHOD("_new", 0, CHPANEL_new, "(Parent)Container;"),

	ARRANGEMENT_FLAG_PROPERTIES,

  HPANEL_DESCRIPTION,

  GB_END_DECLARE
};


GB_DESC CVPanelDesc[] =
{
  GB_DECLARE("VPanel", sizeof(CPANEL)), GB_INHERITS("Container"),

  GB_METHOD("_new", 0, CVPANEL_new, "(Parent)Container;"),

	ARRANGEMENT_FLAG_PROPERTIES,

  VPANEL_DESCRIPTION,

  GB_END_DECLARE
};



