/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://libatres.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com), Boris Mikic                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#include <hltypes/harray.h>
#include <hltypes/hstring.h>

#include "Endianess.h"
#include "SoundBuffer.h"
#include "Source.h"
#include "AudioManager.h"

#ifndef __APPLE__
#include <AL/al.h>
#else
#include <OpenAL/al.h>
#endif

namespace xal
{
/******* CONSTRUCT / DESTRUCT ******************************************/

	SoundBuffer::SoundBuffer(chstr filename, chstr category, chstr prefix) : duration(0.0f),
		sources(harray<xal::Source*>()), Sound()
	{
		this->filename = hstr(filename);
		this->name = prefix + hstr(filename).replace("\\", "/").rsplit("/").pop_back().rsplit(".", 1).pop_front();
		this->category = xal::mgr->getCategoryByName(category);
	}

	SoundBuffer::~SoundBuffer()
	{
		xal::mgr->logMessage("XAL: Destroying sound " + this->name);
	}
	
/******* METHODS *******************************************************/
	
	void SoundBuffer::bindSource(Source* source)
	{
		source->setSound(this);
		this->sources += source;
	}
	
	void SoundBuffer::unbindSource(Source* source)
	{
		source->setSound(NULL);
		this->sources -= source;
	}
	
	void SoundBuffer::lock()
	{
		if (this->sources.size() > 0)
		{
			this->sources[0]->lock();
		}
	}

	void SoundBuffer::unlock()
	{
		if (this->sources.size() > 0)
		{
			this->sources[0]->unlock();
		}
	}

	bool SoundBuffer::isLocked()
	{
		if (this->sources.size() == 0)
		{
			return false;
		}
		return this->sources[0]->isLocked();
	}

	float SoundBuffer::getSampleOffset()
	{
		if (this->getBuffer() == 0 || this->sources.size() == 0)
		{
			return 0;
		}
		return this->sources[0]->getSampleOffset();
	}

	void SoundBuffer::setGain(float gain)
	{
		if (this->getBuffer() != 0 && this->sources.size() > 0)
		{
			this->sources[0]->setGain(gain);
		}
	}

	float SoundBuffer::getGain()
	{
		if (this->getBuffer() == 0 || this->sources.size() == 0)
		{
			return 1;
		}
		return this->sources[0]->getGain();
	}

	bool SoundBuffer::isPlaying()
	{
		return (this->sources.size() > 0 && this->sources[0]->isPlaying());
	}

	bool SoundBuffer::isPaused()
	{
		return (this->sources.size() > 0 && this->sources[0]->isPaused());
	}

	bool SoundBuffer::isFading()
	{
		return (this->sources.size() > 0 && this->sources[0]->isFading());
	}

	bool SoundBuffer::isFadingIn()
	{
		return (this->sources.size() > 0 && this->sources[0]->isFadingIn());
	}

	bool SoundBuffer::isFadingOut()
	{
		return (this->sources.size() > 0 && this->sources[0]->isFadingOut());
	}

	bool SoundBuffer::isLooping()
	{
		return (this->sources.size() > 0 && this->sources[0]->isLooping());
	}

	bool SoundBuffer::isOgg()
	{
		return this->filename.contains(".ogg");
	}

/******* PLAY CONTROLS *************************************************/
	
	Sound* SoundBuffer::play(float fadeTime, bool looping)
	{
		if (this->getBuffer() == 0)
		{
			return NULL;
		}
		Source* source = NULL;
		if (this->sources.size() == 0 || this->sources[0]->isPlaying())
		{
			source = xal::mgr->allocateSource();
			if (source == NULL)
			{
				return NULL;
			}
			this->bindSource(source);
		}
		else
		{
			source = this->sources[0];
		}
		source->play(fadeTime, looping);
		return source;
	}

	Sound* SoundBuffer::replay(float fadeTime, bool looping)
	{
		if (this->getBuffer() == 0)
		{
			return NULL;
		}
		Source* source = NULL;
		if (this->sources.size() == 0)
		{
			source = xal::mgr->allocateSource();
			if (source == NULL)
			{
				return NULL;
			}
			this->bindSource(source);
		}
		else
		{
			source = this->sources[0];
			source->stop();
		}
		source->replay(fadeTime, looping);
		return source;
	}

	void SoundBuffer::stop(float fadeTime)
	{
		if (this->getBuffer() != 0 && this->sources.size() > 0)
		{
			this->sources[0]->stop(fadeTime);
		}
	}

	void SoundBuffer::stopAll(float fadeTime)
	{
		if (this->getBuffer() != 0)
		{
			for (Source** it = this->sources.iterate(); it; it = this->sources.next())
			{
				(*it)->stop(fadeTime);
			}
		}
	}
	
	void SoundBuffer::pause(float fadeTime)
	{
		if (this->getBuffer() != 0 && this->sources.size() > 0)
		{
			this->sources[0]->pause(fadeTime);
		}
	}

}