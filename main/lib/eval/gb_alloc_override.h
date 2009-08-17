/***************************************************************************

  gb_alloc_override.h

  (c) 2000-2009 Benoît Minisini <gambas@users.sourceforge.net>

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#ifndef __GB_ALLOC_H
#define __GB_ALLOC_H

#include "main.h"

#define ALLOC(_ptr, _len, _where) GB.Alloc((void **)(void *)_ptr, _len)
#define FREE(_ptr, _where) GB.Free((void **)(void *)_ptr)
#define REALLOC(_ptr, _len, _where) GB.Realloc((void **)(void *)_ptr, _len)

#endif

