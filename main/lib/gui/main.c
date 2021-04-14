/***************************************************************************

  main.c

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

#define __MAIN_C

#include "main.h"

enum { USE_NOTHING, USE_GB_QT4, USE_GB_QT5, USE_GB_GTK, USE_GB_GTK3 };

static char use_list[4][3] = {
	{ USE_GB_QT5, USE_GB_GTK, USE_GB_GTK3 },
	{ USE_GB_QT4, USE_GB_GTK3, USE_GB_GTK },
	{ USE_GB_GTK3, USE_GB_QT4, USE_GB_QT5 },
	{ USE_GB_GTK, USE_GB_QT5, USE_GB_QT4 },
};

GB_INTERFACE GB EXPORT;

// Prevents gbi3 from complaining

GB_DESC *GB_CLASSES[] EXPORT =
{
  NULL
};

char *GB_INCLUDE EXPORT = "gb.qt4|gb.qt5|gb.gtk|gb.gtk3";

static bool _debug = FALSE;


const char *get_name(int use)
{
	switch (use)
	{
		case USE_GB_QT4: return "gb.qt4";
		case USE_GB_QT5: return "gb.qt5";
		case USE_GB_GTK3: return "gb.gtk3";
		default: return "gb.gtk";
	}
}

#include "gb_gui_test_temp.h"

int EXPORT GB_INIT(void)
{
	int use = USE_NOTHING;
	int use_other = USE_NOTHING;
	char *env;
	const char *comp;
	int i;
	const char *fail;
	char not_found[32];

	env = getenv("GB_GUI");
	if (env)
	{
		if (strcmp(env, "gb.qt4") == 0)
			use = USE_GB_QT4;
		else if (strcmp(env, "gb.qt5") == 0)
			use = USE_GB_QT5;
		else if (strcmp(env, "gb.gtk") == 0)
			use = USE_GB_GTK;
		else if (strcmp(env, "gb.gtk3") == 0)
			use = USE_GB_GTK3;
	}
	
	env = getenv("GB_GUI_DEBUG");
	if (env && strcmp(env, "0")) 
		_debug = TRUE;
	
	if (use == USE_NOTHING)
	{
		use = GUI_should_use();
		if (use == USE_NOTHING)
			use = USE_GB_GTK3;
	}
	
	if (_debug)
		fprintf(stderr, "gb.gui: checking %s...\n", get_name(use));
	
	fail = GUI_can_use(use);
	if (fail)
	{
		strcpy(not_found, fail);
		use_other = USE_NOTHING;
		for (i = 0; i <= 2; i++)
		{
			if (_debug)
				fprintf(stderr, "gb.gui: checking %s...\n", get_name(use_list[use - 1][i]));
			
			if (!GUI_can_use(use_list[use - 1][i]))
			{
				use_other = use_list[use - 1][i];
				break;
			}
		}
		
		if (use_other)
		{
			fprintf(stderr, "gb.gui: warning: '%s' component not found, using '%s' instead\n", not_found, get_name(use_other));
			use = use_other;
		}
		else
		{
			fprintf(stderr, "gb.gui: error: '%s' component not found, unable to find any GUI replacement component\n", not_found);
			exit(1);
		}
	}
	
	comp = get_name(use);
	
	if (GB.Component.Load(comp))
	{
		fprintf(stderr, "gb.gui: error: cannot load component '%s'\n", comp);
		exit(1);
	}
	else
	{
		if (_debug)
			fprintf(stderr, "gb.gui: loading '%s'\n", comp);
	}
  
  setenv("GB_GUI", comp, TRUE);

  return 0;
}

void EXPORT GB_EXIT()
{
}


