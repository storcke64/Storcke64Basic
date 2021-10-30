/***************************************************************************

  litehtml_patch.h

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
 
#ifndef __LITEHTML_PATCH_H
#define __LITEHTML_PATCH_H

#define NO_GAMBAS_CASE_REPLACEMENT
#include "gambas.h"
extern "C" const GB_INTERFACE *GB_PTR;
#define GB (*GB_PTR)

#undef t_strcasecmp
#define t_strcasecmp GB.StrCaseCmp

#undef t_strncasecmp
#define t_strncasecmp GB.StrNCaseCmp

#undef t_tolower
#define t_tolower GB.ToLower

#undef t_isdigit
#define t_isdigit(_c) ((_c) >= '0' && (_c) <= '9')

#undef t_strtod
#define t_strtod my_strtod
extern "C" double my_strtod(const char *nptr, char **endptr);

#endif
