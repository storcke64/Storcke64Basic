/***************************************************************************

  cprinter.h

  (c) 2000-2017 Benoît Minisini <benoit.minisini@gambas-basic.org>

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

#ifndef __CPRINTER_H
#define __CPRINTER_H

#include "main.h"

#include <QPrinter>

typedef
	struct {
		GB_BASE ob;
		QPrinter *printer;
		int page;
		int page_count;
		int duplex;
		bool cancel;
		bool page_count_set;
		bool printing;
	}
	CPRINTER;

#ifndef __CPRINTER_CPP

extern GB_DESC PrinterDesc[];

#else

#define THIS ((CPRINTER *)_object)
#define PRINTER (THIS->printer)

#endif

QSizeF CPRINTER_get_page_size(CPRINTER *_object);

#endif
