/***************************************************************************

  main.cpp

  gb.form.htmlview component

  (c) Benoît Minisini <g4mba5@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#define __MAIN_CPP

#include "c_htmldocument.h"
#include "main.h"

extern "C" {
	
const GB_INTERFACE *GB_PTR EXPORT;
DRAW_INTERFACE DRAW;
GEOM_INTERFACE GEOM;
IMAGE_INTERFACE IMAGE;

double my_strtod(const char *nptr, char **endptr)
{
	GB_VALUE ret;
	
	if (endptr)
	{
		static bool _warning = false;
		if (!_warning)
		{
			fprintf(stderr, "gb.form.htmlview: warning: m_strtod() 'endptr' argument not implemented\n");
			_warning = true;
		}
	}
	
	if (GB.NumberFromString(GB_NB_READ_FLOAT, nptr, strlen(nptr), &ret))
		return 0.0;
	else
		return ret._float.value;
}


GB_DESC *GB_CLASSES[] EXPORT =
{
	HtmlDocumentDesc,
	NULL
};

int EXPORT GB_INIT(void)
{
	//GB.Component.Load("gb.draw");
  GB.GetInterface("gb.draw", DRAW_INTERFACE_VERSION, &DRAW);
	//GB.Component.Load("gb.geom");
  GB.GetInterface("gb.geom", GEOM_INTERFACE_VERSION, &GEOM);
	//GB.Component.Load("gb.image");
  GB.GetInterface("gb.image", IMAGE_INTERFACE_VERSION, &IMAGE);
	return 0;
}

void EXPORT GB_EXIT()
{
}

}
