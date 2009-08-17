/***************************************************************************

  main.c

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

#define __MAIN_C

#include "c_dbusvariant.h"
#include "c_dbusconnection.h"
#include "c_dbus.h"

#include "main.h"

GB_INTERFACE GB EXPORT;

GB_CLASS CLASS_DBusVariant;

GB_DESC *GB_CLASSES[] EXPORT =
{
	CDBusVariantDesc,
  CDBusConnectionDesc,
  CDBusDesc,
  NULL
};

int EXPORT GB_INIT(void)
{
	CLASS_DBusVariant = GB.FindClass("DBusVariant");
	return FALSE;
}

void EXPORT GB_EXIT()
{
}

