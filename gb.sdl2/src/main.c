/***************************************************************************

  main.c

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

#define __MAIN_C

// Lazyfoo!

#include "gambas.h"
#include "main.h"
#include "c_image.h"
#include "c_draw.h"
#include "c_window.h"
#include "c_mouse.h"
#include "c_font.h"

#include "gb_list_temp.h"

GB_INTERFACE GB EXPORT;
IMAGE_INTERFACE IMAGE EXPORT;
GEOM_INTERFACE GEOM EXPORT;

GB_CLASS CLASS_Window;
GB_CLASS CLASS_Image;
GB_CLASS CLASS_Font;

//-------------------------------------------------------------------------

static void init_sdl()
{
	uint init = SDL_WasInit(SDL_INIT_EVERYTHING);
	const char *error;

	// if audio is defined, sdl was init by gb.sdl2.audio component !
	if (init & SDL_INIT_AUDIO)
	{
		if (SDL_InitSubSystem(SDL_INIT_VIDEO)) // | SDL_INIT_JOYSTICK))
		{
			error = SDL_GetError();
			goto __ERROR;
		}
	}
	else
	{
 		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER)) // | SDL_INIT_JOYSTICK))
		{
			error = SDL_GetError();
			goto __ERROR;
		}
	}

	if (IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG) != (IMG_INIT_JPG | IMG_INIT_PNG))
	{
		error = IMG_GetError();
		goto __ERROR;
	}

	return;

__ERROR:

	fprintf(stderr, "gb.sdl2: unable to initialize SDL2: %s\n", error);
	abort();
}

static void exit_sdl()
{
	uint init;

	if (TTF_WasInit())
		TTF_Quit();

	IMG_Quit();

	init = SDL_WasInit(SDL_INIT_EVERYTHING);

	// if audio is defined, gb.sdl2.audio component still not closed !
	if (init & SDL_INIT_AUDIO)
		SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
	else
		SDL_Quit();
}

static bool event_loop()
{
	SDL_Event event;
	bool ret = FALSE;
	
	while (SDL_PollEvent(&event))
	{
		switch(event.type)
		{
			case SDL_QUIT:
				ret = TRUE;
				break;
			case SDL_WINDOWEVENT:
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
			case SDL_MOUSEMOTION:
			case SDL_MOUSEWHEEL:
			case SDL_KEYDOWN:
			case SDL_KEYUP:
			case SDL_TEXTINPUT:
				WINDOW_handle_event(&event);
				ret = TRUE;
				break;
		}
	}
	
	return ret;
}

static void my_main(int *argc, char **argv)
{
	char *env;
	const char *driver = NULL;
	
	env = getenv("GB_GUI_PLATFORM");
	if (env && *env)
	{
		if (!strcasecmp(env, "wayland"))
			driver = "SDL_VIDEODRIVER=wayland";
		else if (!strcasecmp(env, "x11"))
			driver = "SDL_VIDEODRIVER=x11";
		else
			fprintf(stderr, "gb.sdl2: warning: unsupported platform: %s\n", env);
	}
	
	if (!driver)
	{
		if (getenv("WAYLAND_DISPLAY"))
			putenv("SDL_VIDEODRIVER=wayland");
	}
	else
		putenv((char *)driver);
	
	init_sdl();

	CLASS_Window = GB.FindClass("Window");
	CLASS_Image = GB.FindClass("Image");
	CLASS_Font = GB.FindClass("Font");
}

static int my_loop()
{
	for(;;)
	{
		if (!GB.Loop(10) && !WINDOW_list)
			break;
		event_loop();
		WINDOW_update();
	}

	return 1;
}

static void my_wait(int duration)
{
	if (duration >= 0)
	{
		GB.Loop(10);
		event_loop();
		WINDOW_update();
	}
	else if (duration == -1)
	{
		GB.Loop(10);
		WINDOW_update();
	}
	else if (duration == -2)
	{
		while (!GB.Loop(10) && !event_loop())
			WINDOW_update();
	}
}

//-------------------------------------------------------------------------

GB_DESC *GB_CLASSES[] EXPORT =
{
	ImageDesc,
	DrawDesc,
	WindowDesc,
	KeyDesc,
	MouseDesc,
	FontDesc,
	NULL
};

int EXPORT GB_INIT(void)
{
	GB.Component.Load("gb.geom");
  GB.GetInterface("gb.geom", GEOM_INTERFACE_VERSION, &GEOM);
	GB.Component.Load("gb.image");
  GB.GetInterface("gb.image", IMAGE_INTERFACE_VERSION, &IMAGE);
	IMAGE.SetDefaultFormat(GB_IMAGE_BGRA);

	GB.Hook(GB_HOOK_MAIN, (void *)my_main);
	GB.Hook(GB_HOOK_LOOP, (void *)my_loop);
	GB.Hook(GB_HOOK_WAIT, (void *)my_wait);

	return -1;
}

void EXPORT GB_EXIT()
{
	exit_sdl();
}

void EXPORT GB_SIGNAL(int signal, void *param)
{
	//static bool wasFullscreen = false;

	//if (!SDLcore::GetWindow())
	//	return;

	if ((signal == GB_SIGNAL_DEBUG_BREAK) || (signal == GB_SIGNAL_DEBUG_CONTINUE))
	{
		/*if (SDLcore::GetWindow()->IsFullScreen())
		{
			wasFullscreen = true;
			SDLcore::GetWindow()->SetFullScreen(false);
		}*/
	}

	if (signal == GB_SIGNAL_DEBUG_CONTINUE)
	{
		/*if (wasFullscreen)
			SDLcore::GetWindow()->SetFullScreen(true);*/
	}
}

