/***************************************************************************

  CPrinter.h

  (c) 2000-2005 Beno�t Minisini <gambas@users.sourceforge.net>

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

#ifndef __CPRINTER_H
#define __CPRINTER_H

#include "gambas.h"

#include <qprinter.h>

#include "CWidget.h"

#ifndef __CPRINTER_CPP

extern GB_DESC CPrinterDesc[];
extern QPrinter *CPRINTER_printer;

#else

#define PRINTER  CPRINTER_printer

typedef
  struct {
    char *paper;
    QPrinter::PageSize value;
    }
  PRINTER_SIZE;

#endif

void CPRINTER_init(void);

#endif
