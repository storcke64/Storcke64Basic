/***************************************************************************

  CScreen.h

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

#ifndef __CSCREEN_H
#define __CSCREEN_H

#include "main.h"
#include "gdesktop.h"
#include "gapplication.h"

#ifndef __CSCREEN_CPP
extern GB_DESC ScreenDesc[];
extern GB_DESC ScreensDesc[];
extern GB_DESC DesktopDesc[];
extern GB_DESC ApplicationDesc[];

extern char *CAPPLICATION_Theme;
extern GB_ARRAY CAPPLICATION_Restart;
extern bool CAPPLICATION_MiddleClickPaste;

#else

#define SCREEN ((CSCREEN *)_object)

#endif

typedef
	struct
	{
		GB_BASE ob;
		int index;
	}
	CSCREEN;
	
#endif
