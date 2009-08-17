/***************************************************************************

  main.h

  (c) 2006 Laurent Carlier <lordheavy@users.sourceforge.net>

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

#ifndef __MAIN_H
#define __MAIN_H

#include "gambas.h"
#include "gb.image.h"
#include "SDLerror.h"

#define COMP_WARN	"gb.sdl warning: ",
#define COMP_ERR	"gb.sdl error: ",
#define COMP_INFO	"gb.sdl info: ",

#ifndef __MAIN_CPP
extern GB_INTERFACE GB;
extern IMAGE_INTERFACE IMAGE;
extern GB_CLASS CLASS_Window;
extern GB_CLASS CLASS_Image;
#endif


#endif /* __MAIN_H */

