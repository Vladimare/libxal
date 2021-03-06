/// @file
/// @version 3.4
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Represents an implementation of the AudioManager for SDL.

#ifdef _SDL
#ifndef XAL_SDL_AUDIO_MANAGER_H
#define XAL_SDL_AUDIO_MANAGER_H

#include <SDL/SDL.h>

#include <hltypes/hstream.h>
#include <hltypes/hstring.h>

#include "AudioManager.h"
#include "xalExport.h"

#define SDL_MAX_PLAYING 32

namespace xal
{
	class SDL_Player;
	class Player;
	class Sound;
	class Source;

	class xalExport SDL_AudioManager : public AudioManager
	{
	public:
		friend class SDL_Player;

		SDL_AudioManager(void* backendId, bool threaded = false, float updateTime = 0.01f, chstr deviceName = "");
		~SDL_AudioManager();

		inline SDL_AudioSpec getFormat() { return this->format; }

		void mixAudio(void* unused, unsigned char* stream, int length);

	protected:
		SDL_AudioSpec format;
		hstream buffer;

		Player* _createSystemPlayer(Sound* sound);

		static void _mixAudio(void* unused, unsigned char* stream, int length);

		void _convertStream(Source* source, hstream& stream);
		
	};

}

#endif
#endif