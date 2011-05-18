/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#if HAVE_OGG
#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#include "Endianess.h"
#include "AudioManager.h"
#include "OGG_Source.h"
#include "xal.h"

namespace xal
{
	OGG_Source::OGG_Source(chstr filename) : Source(filename)
	{
	}

	OGG_Source::~OGG_Source()
	{
	}

	bool OGG_Source::open()
	{
		bool result = Source::open();
		if (result)
		{
			if (ov_fopen((char*)this->filename.c_str(), &this->oggStream) == 0)
			{
				vorbis_info* info = ov_info(&oggStream, -1);
				this->channels = info->channels;
				this->samplingRate = info->rate;
				this->bitsPerSample = 16; // always 16 bit data
				int bytes = this->bitsPerSample / 8;
				this->size = (int)ov_pcm_total(&this->oggStream, -1) * this->channels * bytes;
				this->duration = ((float)this->size) / (this->samplingRate * this->channels * bytes);
			}
			else
			{
				xal::log("ogg: error opening file!");
				result = false;
			}
		}
		return result;
	}

	bool OGG_Source::close()
	{
		bool result = Source::close();
		if (result)
		{
			ov_clear(&this->oggStream);
		}
		return result;
	}

	bool OGG_Source::rewind()
	{
		bool result = Source::rewind();
		if (result)
		{
			ov_raw_seek(&this->oggStream, 0);
		}
		return result;
	}

	bool OGG_Source::load(unsigned char* output)
	{
		if (!Source::load(output))
		{
			return false;
		}
		int section;
		unsigned long remaining = this->size;
		unsigned char* buffer = output;
		int read;
		while (remaining > 0)
		{
			read = ov_read(&this->oggStream, (char*)buffer, remaining, 0, 2, 1, &section);
			if (read == 0)
			{
				break;
			}
			remaining -= read;
			buffer += read;
		}
#ifdef __BIG_ENDIAN__ // TODO - this should be tested properly
		for (int i = 0; i < this->size; i += 2)
		{
			XAL_NORMALIZE_ENDIAN(*(uint16_t*)(output + i)); // always 16 bit data
		}
#endif	
		return true;
	}

	int OGG_Source::loadChunk(unsigned char* output, int size)
	{
		if (Source::loadChunk(output, size) == 0)
		{
			return 0;
		}
		int remaining = size;
		int section;
		unsigned char* buffer = output;
		int read;
		while (remaining > 0)
		{
			read = ov_read(&this->oggStream, (char*)buffer, remaining, 0, 2, 1, &section);
			if (read == 0)
			{
				break;
			}
			remaining -= read;
			buffer += read;
		}
#ifdef __BIG_ENDIAN__ // TODO - this should be tested properly
		for (int i = 0; i < this->size; i += 2)
		{
			XAL_NORMALIZE_ENDIAN(*(uint16_t*)(output + i)); // always 16 bit data
		}
#endif	
		return (size - remaining);
	}

}
#endif