/***************************************************************************

  gbx_regexp.h

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

#ifndef __GBX_REGEXP_H
#define __GBX_REGEXP_H

#include "gbx_c_array.h"

typedef
	void (*REGEXP_SCAN_FUNC)(const char *, int);

bool REGEXP_match(const char *pattern, int len_pattern, const char *string, int len_string);
bool REGEXP_match_pcre(const char *pattern, int len_pattern, const char *string, int len_string);
bool REGEXP_scan(CARRAY *array, const char *pattern, int len_pattern, const char *string, int len_string);

#endif

