/***************************************************************************

  gb_component.h

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

#ifndef __GB_COMPONENT_H
#define __GB_COMPONENT_H

#include "config.h"

typedef
  struct {
    int version;
    void *func[0];
    }
  INTERFACE;

#define LIB_INIT         "GB_INIT"
#define LIB_EXIT         "GB_EXIT"
#define LIB_INTERFACE    "GB_INTERFACE"
#define LIB_CLASS        "GB_CLASSES"
#define LIB_INCLUDE      "GB_INCLUDE"
#define LIB_SIGNAL       "GB_SIGNAL"
#define LIB_INFO         "GB_INFO"
#define LIB_GAMBAS       "GB"

#ifdef DONT_USE_LTDL
 #ifdef OS_OPENBSD
 #define LIB_PATTERN    "%s/%s." SHARED_LIBRARY_EXT ".0.0"
 #else
 #define LIB_PATTERN    "%s/%s." SHARED_LIBRARY_EXT
 #endif
#else
#define LIB_PATTERN    "%s/%s.la"
#endif

#define ARCH_PATTERN   "%s/%s.gambas"

#define GAMBAS_LINK_PATH  "/usr/bin/gbx" GAMBAS_VERSION_STRING

#define GAMBAS_LIB_PATH "lib/gambas" GAMBAS_VERSION_STRING
#define GAMBAS_LIB64_PATH "lib64/gambas" GAMBAS_VERSION_STRING

#endif
