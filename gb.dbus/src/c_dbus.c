/***************************************************************************

	c_dbus.c

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

#define __C_DBUS_C

#include "helper.h"
#include "c_dbusconnection.h"
#include "c_dbus.h"

BEGIN_PROPERTY(DBus_Debug)

	if (READ_PROPERTY)
		GB.ReturnBoolean(DBUS_Debug);
	else
		DBUS_Debug = VPROP(GB_BOOLEAN);

END_PROPERTY

BEGIN_METHOD(DBus_SplitSignature, GB_STRING sign)

	GB.ReturnObject(DBUS_split_signature(GB.ToZeroString(ARG(sign))));

END_METHOD

GB_DESC CDBusDesc[] =
{
	GB_DECLARE_STATIC("_DBus"),

	GB_STATIC_PROPERTY("Debug", "b", DBus_Debug),

	GB_CONSTANT("Method", "i", DBUS_MESSAGE_TYPE_METHOD_CALL),
	GB_CONSTANT("Reply", "i", DBUS_MESSAGE_TYPE_METHOD_RETURN),
	GB_CONSTANT("Signal", "i", DBUS_MESSAGE_TYPE_SIGNAL),
	GB_CONSTANT("Error", "i", DBUS_MESSAGE_TYPE_ERROR),
	
	GB_STATIC_METHOD("_SplitSignature", "String[]", DBus_SplitSignature, "(Signature)s"),
	
	GB_END_DECLARE
};

