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

#include "gambas.h"
#include "main.h"
#include "c_channel.h"
#include "c_music.h"
#include "c_sound.h"

GB_INTERFACE GB EXPORT;

GB_CLASS CLASS_Sound;
GB_CLASS CLASS_Channel;

int AUDIO_frequency = 44100;
int AUDIO_buffer_size = 4096;
bool AUDIO_initialized = FALSE;

//-------------------------------------------------------------------------

static void init_mixer(int flag)
{
	if ((Mix_Init(flag) & flag) != flag)
		fprintf(stderr, "gb.sdl2.audio: warning: %s\n", Mix_GetError());
}

bool AUDIO_init()
{
	Uint16 format;
	int channels;

	if (AUDIO_initialized)
		return FALSE;

	init_mixer(MIX_INIT_MP3);
	init_mixer(MIX_INIT_OGG);
	init_mixer(MIX_INIT_MOD);
	init_mixer(MIX_INIT_FLAC);
#ifdef MIX_INIT_FLUIDSYNTH
	init_mixer(MIX_INIT_FLUIDSYNTH);
#endif
#ifdef MIX_INIT_MID
	init_mixer(MIX_INIT_MID);
#endif

	if (Mix_OpenAudio(AUDIO_frequency, MIX_DEFAULT_FORMAT, 2, AUDIO_buffer_size))
	{
		GB.Error("Unable to initialize mixer");
		return TRUE;
	}

	Mix_QuerySpec(&AUDIO_frequency, &format, &channels);
	//fprintf(stderr, "AUDIO_init: %d %d %d\n", AUDIO_frequency, format, channels);

	if (CHANNEL_init())
		return TRUE;

	AUDIO_initialized = TRUE;
	return FALSE;
}

static void AUDIO_exit()
{
	if (!AUDIO_initialized)
		return;

	// Don't free Gambas objects from GB_EXIT!
	// CHANNEL_exit(); 
	MUSIC_exit();
	Mix_CloseAudio();

	while (Mix_Init(0))
		Mix_Quit();
}

static void init_sdl()
{
	uint init = SDL_WasInit(SDL_INIT_EVERYTHING);

	// if video is defined, SDL was initialized by gb.sdl2 component !
	if (init & SDL_INIT_VIDEO)
	{
		if (SDL_InitSubSystem(SDL_INIT_AUDIO))
			goto __ERROR;
	}
	else
	{
		if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER))
			goto __ERROR;
	}

	return;

__ERROR:

	fprintf(stderr, "gb.sdl2.audio: unable to initialize SDL: %s\n", SDL_GetError());
	abort();
}

static void exit_sdl()
{
	uint init = SDL_WasInit(SDL_INIT_EVERYTHING);

	AUDIO_exit();

	// if video is defined, gb.sdl2 component still not closed !
	if (init & SDL_INIT_VIDEO)
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
	else
		SDL_Quit();
}

//-------------------------------------------------------------------------

GB_DESC *GB_CLASSES[] EXPORT =
{
	SoundDesc,
	ChannelDesc,
	ChannelsDesc,
	MusicDesc,
	NULL
};

int EXPORT GB_INIT(void)
{
	CLASS_Sound = GB.FindClass("Sound");
	CLASS_Channel = GB.FindClass("Channel");

	init_sdl();

	return -1;
}

void EXPORT GB_EXIT()
{
	exit_sdl();
}
