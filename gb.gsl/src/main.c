/***************************************************************************

  main.c

  gb.gsl component

  (c) 2012 Randall Morgan <rmorgan62@gmail.com>

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

#define __MAIN_C

#include "main.h"
#include "c_gsl.h"
#include "gambas.h"
#include "gb_common.h"


#ifdef __cplusplus
extern "C" {
#endif

GB_INTERFACE GB EXPORT;

GB_DESC *GB_CLASSES[] EXPORT =
{
     CGslDesc,
     NULL 
};

int EXPORT GB_INIT(void)
{
	//CLASS_GSL = GB.FindClass("CGsl");
	return 0;
}

void EXPORT GB_EXIT()
{
}



#ifdef _cpluscplus
}
#endif
