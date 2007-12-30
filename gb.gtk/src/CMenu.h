/***************************************************************************

  CMenu.h

  (c) 2004-2005 - Daniel Campos Fernández <dcamposf@gmail.com>
  
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

#ifndef __CMENU_H
#define __CMENU_H

#include "gambas.h"
#include "widgets.h"
#include "CPicture.h"

#ifndef __CMENU_CPP
extern GB_DESC CMenuDesc[];
extern GB_DESC CMenuChildrenDesc[];
#else

#define THIS  ((CMENU*)_object)
#define MENU  ((gMenu*)THIS->widget)
#define CMENU_PROPERTIES "Text,Picture,Enabled,Checked,Visible,Tag,Shortcut"

#endif

typedef struct _CMENU
{
	GB_BASE ob;
	gMenu *widget;
	GB_VARIANT_VALUE tag;

	CPICTURE *picture;
  
} CMENU;




#endif
