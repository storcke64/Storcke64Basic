/***************************************************************************

  gb_gui_test_temp.h

  (c) 2000-2020 Benoît Minisini <g4mba5@gmail.com>

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

#define __GB_GUI_TEST_TEMP_H

static char *make_name(const char *prefix, const char *suffix)
{
	static char buffer[32];
	
	snprintf(buffer, sizeof(buffer), "%s.%s", prefix, suffix);
	return buffer;
}

static bool GUI_can_use(int use)
{
	static const char *ext[] = { "ext", "webkit", "opengl", NULL };
	const char **pext;
	const char *name;
	
	name = get_name(use);
	
	if (!GB.Component.CanLoadLibrary(name))
		return FALSE;

	for (pext = ext; *pext; pext++)
	{
		if (GB.Component.Exist(make_name("gb.gui", *pext)) && !GB.Component.CanLoadLibrary(make_name(name, *pext)))
			return FALSE;
	}
	
	return TRUE;
}


 
