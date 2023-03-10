/***************************************************************************

  crect.c

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

#define __CRECT_C

#include "gb_common.h"
#include "cpoint.h"
#include "crect.h"
#include "crect_temp.h"

IMPLEMENT_RECT_CLASS(CRECT, Rect, GB_INTEGER, int, "i", GB.ReturnInteger, ((CRECT *)_object), CPOINT, Point)
IMPLEMENT_RECT_CLASS(CRECTF, RectF, GB_FLOAT, double, "f", GB.ReturnFloat, ((CRECTF *)_object), CPOINTF, PointF)
