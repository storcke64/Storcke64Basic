/***************************************************************************

  c_system.c

  (c) 2000-2017 Benoît Minisini <g4mba5@gmail.com>

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

#define __C_SYSTEM_C

#include "gambas.h"
#include "gb_common.h"
#include "gb_reserved.h"
#include "c_system.h"

static GB_ARRAY _keywords = 0;
static GB_ARRAY _datatypes = 0;
static GB_ARRAY _subroutines = 0;

BEGIN_PROPERTY(System_Keywords)

	COMP_INFO *info;
	char *str;

	if (!_keywords)
	{
	  GB.Array.New(&_keywords, GB_T_STRING, 0);

	  for (info = &COMP_res_info[1]; info->name; info++)
	  {
	  	if (*info->name >= 'A' && *info->name <= 'Z')
	  	{
      	str = GB.NewZeroString(info->name);
      	*((char **)GB.Array.Add(_keywords)) = str;
			}
		}

		GB.Array.SetReadOnly(_keywords);
		GB.Ref(_keywords);
	}

	GB.ReturnObject(_keywords);

END_PROPERTY

BEGIN_PROPERTY(System_Subroutines)

	SUBR_INFO *subr;
	char *str;

	if (!_subroutines)
	{
	  GB.Array.New(&_subroutines, GB_T_STRING, 0);

  	for (subr = &COMP_subr_info[0]; subr->name; subr++)
	  {
      str = GB.NewZeroString(subr->name);
      *((char **)GB.Array.Add(_subroutines)) = str;
		}

		GB.Array.SetReadOnly(_subroutines);
		GB.Ref(_subroutines);
	}

	GB.ReturnObject(_subroutines);

END_PROPERTY

BEGIN_PROPERTY(System_Datatypes)

	COMP_INFO *info;
	char *str;

	if (!_datatypes)
	{
	  GB.Array.New(&_datatypes, GB_T_STRING, 0);

	  for (info = &COMP_res_info[1]; info->name; info++)
	  {
	  	if (info->flag & RSF_TYPE)
	  	{
      	str = GB.NewZeroString(info->name);
      	*((char **)GB.Array.Add(_datatypes)) = str;
			}
		}

		GB.Array.SetReadOnly(_datatypes);
		GB.Ref(_datatypes);
	}

	GB.ReturnObject(_datatypes);

END_PROPERTY

BEGIN_METHOD_VOID(CSYSTEM_exit)

	GB.Unref((void **)&_keywords);
	GB.Unref((void **)&_datatypes);
	GB.Unref((void **)&_subroutines);

END_METHOD


GB_DESC CSystemDesc[] =
{
  GB_DECLARE("System", 0),

	GB_STATIC_METHOD("_exit", NULL, CSYSTEM_exit, NULL),

  GB_STATIC_PROPERTY_READ("Keywords", "String[]", System_Keywords),
  GB_STATIC_PROPERTY_READ("Datatypes", "String[]", System_Datatypes),
  GB_STATIC_PROPERTY_READ("Subroutines", "String[]", System_Subroutines),

  GB_END_DECLARE
};

