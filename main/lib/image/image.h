/***************************************************************************

  image.h

  (c) 2000-2007 Benoit Minisini <gambas@users.sourceforge.net>

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

#ifndef __IMAGE_H
#define __IMAGE_H

#include "main.h"

static inline int RED(uint rgba) { return ((rgba >> 16) & 0xff); }
static inline int GREEN(uint rgba) { return ((rgba >> 8) & 0xff); }
static inline int BLUE(uint rgba) { return (rgba & 0xff); }
static inline int ALPHA(uint rgba) { return ((rgba >> 24) & 0xff); }
static inline uint RGB(int r, int g, int b) { return (0xff << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff); }
static inline uint RGBA(int r, int g, int b, int a) { return ((a & 0xff) << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff); }
static inline int GRAY(uint rgba) { return (RED(rgba) * 11 + GREEN(rgba) * 16 + BLUE(rgba) * 5) / 32; }

int IMAGE_size(GB_IMG *img);
void IMAGE_create(GB_IMG *img, int width, int height, int format);
void IMAGE_create_with_data(GB_IMG *img, int width, int height, int format, unsigned char *data);
void IMAGE_take(GB_IMG *img, GB_IMG_OWNER *owner, void *owner_handle, int width, int height, unsigned char *data);
void *IMAGE_check(GB_IMG *img, GB_IMG_OWNER *temp_owner);
void IMAGE_delete(GB_IMG *img);
void IMAGE_convert(GB_IMG *img, int format);
void IMAGE_fill(GB_IMG *img, GB_COLOR col);
void IMAGE_make_gray(GB_IMG *img);
void IMAGE_make_transparent(GB_IMG *img, GB_COLOR color);
GB_COLOR IMAGE_get_pixel(GB_IMG *img, int x, int y);
void IMAGE_set_pixel(GB_IMG *img, int x, int y, GB_COLOR col);
void IMAGE_replace(GB_IMG *img, GB_COLOR src, GB_COLOR dst, bool noteq);
void IMAGE_set_default_format(int format);
int IMAGE_get_default_format();

#endif
