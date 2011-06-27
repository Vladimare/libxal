/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://code.google.com/p/libxal/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com), Boris Mikic,                    *
			       Ivan Vucica (ivan@vucica.net)                                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#include <hltypes/hstring.h>
#include "Category.h"
#include "SourceApple.h"
#include "SoundBuffer.h"
#include "StreamSound.h"
#include "AudioManager.h"

#ifndef __APPLE__
	#include <AL/al.h>
#else
	#include <OpenAL/al.h>
#endif

#import <AVFoundation/AVFoundation.h>

#define avAudioPlayer ((AVAudioPlayer*)this->avAudioPlayer_Void)
#define avAudioPlayerDelegate ((XAL_AVAudioPlayer_Delegate*)this->avAudioPlayerDelegate_Void)

@interface XAL_AVAudioPlayer_Delegate : NSObject <AVAudioPlayerDelegate>
{
	xal::SourceApple *sourceApple;
	BOOL playing;
	BOOL usingSymlink;
}

- (void)markAsPlaying;
- (void)audioPlayerDidFinishPlaying:(AVAudioPlayer *)player successfully:(BOOL)flag;
@property (readonly, getter=isPlaying) BOOL playing;
@property (assign, getter=isUsingSymlink) BOOL usingSymlink;
@end

@implementation XAL_AVAudioPlayer_Delegate
@synthesize playing;
@synthesize usingSymlink;
- (id)initWithOwner:(xal::SourceApple*)_sourceApple
{
	if(self=[super init])
	{
		sourceApple = _sourceApple;
		usingSymlink = NO;
	}
	return self;
}

- (void)markAsPlaying
{
	playing = YES;
}
- (void)audioPlayerDidFinishPlaying:(AVAudioPlayer *)player successfully:(BOOL)flag
{
	playing = NO;
}
- (void)audioPlayerDecodeErrorDidOccur:(AVAudioPlayer *)player error:(NSError *)error;
{
	int code = [error code];
	NSLog(@"decode error at time %g with code %d: %@", player.currentTime, code, error);
	NSLog(@"user dict: %@ recovery suggestion: %@", [error userInfo], [error localizedRecoverySuggestion]);
	if (code == -50) {
		NSLog(@"skipping player to appropriate time");
		
		AVAudioPlayer* avp = (AVAudioPlayer*)sourceApple->_rebuildPlayer();
		[avp play];
	}
	else
	{
		NSLog(@"code is not -50");
		playing = NO;
	}
}

@end




namespace xal
{
/******* CONSTRUCT / DESTRUCT ******************************************/

	SourceApple::SourceApple(SoundBuffer* sound, unsigned int sourceId) : Sound(),
		gain(1.0f), looping(false), paused(false), fadeTime(0.0f),
		fadeSpeed(0.0f), bound(true), sampleOffset(0.0f)
	{
		this->sound = sound;
		this->sourceId = sourceId;
		
		NSURL *url = [NSURL fileURLWithPath:[NSString stringWithUTF8String:sound->getVirtualFileName().c_str()]];
		this->avAudioPlayer_Void = [[AVAudioPlayer alloc] initWithContentsOfURL:url 
																		  error:nil];
		this->avAudioPlayerDelegate_Void = [[XAL_AVAudioPlayer_Delegate alloc] initWithOwner:this];
		
		avAudioPlayer.delegate = avAudioPlayerDelegate;
		
	}

	SourceApple::~SourceApple()
	{
		[avAudioPlayer release];
		[avAudioPlayerDelegate release];
	
	}

/******* METHODS *******************************************************/
	
	void* SourceApple::_rebuildPlayer()
	{
		if (avAudioPlayer) {
			
			float volume = avAudioPlayer.volume;
			int numberOfLoops = avAudioPlayer.numberOfLoops;
			float currentTime = avAudioPlayer.currentTime;
			XAL_AVAudioPlayer_Delegate* delegate = avAudioPlayer.delegate;
			
			[avAudioPlayer stop];
			[avAudioPlayer release];

			// run one mainloop step
			SInt32 result;
			do {
				result = CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, TRUE);
			} while(result == kCFRunLoopRunHandledSource);
			
			
			delegate.usingSymlink = !delegate.usingSymlink;
			NSString *path = [NSString stringWithUTF8String:sound->getVirtualFileName().c_str()];
			
			if(delegate.usingSymlink)
			{
				NSString *ext = [path pathExtension];
				NSString *extless = [path substringToIndex:path.length - ext.length];
				path = [extless stringByAppendingString:@"2.m4a"];
			}
			
			
			
			
			NSURL *url = [NSURL fileURLWithPath:path];
			this->avAudioPlayer_Void = [[AVAudioPlayer alloc] initWithContentsOfURL:url error:nil];
			avAudioPlayer.volume = volume;
			avAudioPlayer.numberOfLoops = numberOfLoops;
			avAudioPlayer.currentTime = currentTime;
			avAudioPlayer.delegate = delegate;
			
			//this->stopSoft(1, false);
			
		}
		return avAudioPlayer;
	}
	void* SourceApple::_getPlayerAsVoidPtr() const
	{
		return this->avAudioPlayer_Void;
	}
	
	void SourceApple::update(float k)
	{
		//if (this->sourceId == 0)
		//{
		//	return;
		//}
		
		if (this->isPlaying())
		{
			if (this->isFading())
			{
				this->fadeTime += this->fadeSpeed * k;
				if (this->fadeTime >= 1.0f && this->fadeSpeed > 0.0f)
				{
					avAudioPlayer.volume = this->gain * this->sound->getCategory()->getGain() * xal::mgr->getGlobalGain();
					
					this->fadeTime = 1.0f;
					this->fadeSpeed = 0.0f;
				}
				else if (this->fadeTime <= 0.0f && this->fadeSpeed < 0.0f)
				{
					this->fadeTime = 0.0f;
					this->fadeSpeed = 0.0f;
					if (!this->paused)
					{
						this->stop();
						return;
					}
					this->pause();
					return;
				}
				else
				{
					avAudioPlayer.volume = this->fadeTime * this->gain * this->sound->getCategory()->getGain() * xal::mgr->getGlobalGain();
					
				}
			}
		}
		if (!this->isPlaying() && !this->isPaused())
		{
			this->unbind(false);
		}
	}

	Sound* SourceApple::play(float fadeTime, bool looping)
	{
		if (this->sourceId == 0)
		{
			this->sourceId = xal::mgr->allocateSourceId();
			if (this->sourceId == 0)
			{
				NSLog(@"Allocation of source failed");
				return NULL;
			}
#ifdef _DEBUG
			xal::log(hsprintf("allocated source ID %d", this->sourceId));
#endif
		}
#ifdef _DEBUG
		xal::log("play sound " + this->sound->getVirtualFileName());
#endif
		if (!this->paused)
		{
			this->looping = looping;
		}
		bool alreadyFading = this->isFading();
		if (!alreadyFading)
		{
			// if set to -1, we will loop indefinitely. if set to 0, no repeats.
			avAudioPlayer.numberOfLoops = (this->looping ? -1 : 0); 
			
			if (this->isPaused())
			{
				avAudioPlayer.currentTime = this->sampleOffset;
			}
		}
		if (fadeTime > 0.0f)
		{
			this->fadeSpeed = 1.0f / fadeTime;
#ifdef _DEBUG
			xal::log("fading in sound " + this->sound->getVirtualFileName());
#endif
		}
		else
		{
			this->fadeTime = 1.0f;
			this->fadeSpeed = 0.0f;
		}
		avAudioPlayer.volume = this->fadeTime * this->gain *
			this->sound->getCategory()->getGain() * xal::mgr->getGlobalGain();
		if (!alreadyFading)
		{
			//alSourcePlay(this->sourceId);
			if(!this->isPlaying())
			{
				[avAudioPlayer play];
				[avAudioPlayerDelegate markAsPlaying];
			}
		}
		this->paused = false;
		return this;
	}

	void SourceApple::stop(float fadeTime)
	{
		this->stopSoft(fadeTime);
		if (/*this->sourceId != 0 && */fadeTime <= 0.0f)
		{
			
			[avAudioPlayer stop];

			this->unbind(this->paused);
		}
	}

	void SourceApple::pause(float fadeTime)
	{
		this->stopSoft(fadeTime, true);
		if (/*this->sourceId != 0 && */fadeTime <= 0.0f)
		{
			[avAudioPlayer stop];

			this->unbind(this->paused);
		}
	}

	void SourceApple::stopSoft(float fadeTime, bool pause)
	{

		/*if (this->sourceId == 0)
		{
			NSLog(@"Bailing out of stopSoft; sourceId is 0");
			return;
		}*/
		this->paused = pause;
		if (fadeTime > 0.0f)
		{
#ifdef _DEBUG
			xal::log("fading out sound " + this->sound->getVirtualFileName());
#endif
			this->fadeSpeed = -1.0f / fadeTime;
			return;
		}
#ifdef _DEBUG
		xal::log(hstr(this->paused ? "pause" : "stop") + " sound " + this->sound->getVirtualFileName());
#endif
		this->fadeTime = 0.0f;
		this->fadeSpeed = 0.0f;
		//alGetSourcef(this->sourceId, AL_SEC_OFFSET, &this->sampleOffset);
		//alSourceStop(this->sourceId);
		this->sampleOffset = avAudioPlayer.currentTime;
		[avAudioPlayer stop];
		//this->_rebuildPlayer();
	}

	void SourceApple::unbind(bool pause)
	{
		if (!this->isLocked())
		{
			this->sourceId = 0;
			if (!pause)
			{
				[avAudioPlayer stop];
				this->sound->unbindSource(this);
				xal::mgr->destroySource(this);
			}
			else
			{
				// FIXME WAAH we are not just pausing here
				// however, we must call destroySource, else when
				// in SoundBuffer this->sources[0] is accessed
				// it'll still call functions of the "old", paused
				// source
				[avAudioPlayer stop];
				this->sound->unbindSource(this);
				xal::mgr->destroySource(this);

			}
		}
	}
	
/******* PROPERTIES ****************************************************/

	void SourceApple::setGain(float gain)
	{
		this->gain = gain;
		/*if (this->sourceId != 0)
		{*/
			avAudioPlayer.volume = this->gain *
				this->sound->getCategory()->getGain() * xal::mgr->getGlobalGain();
		//}
	}

	unsigned int SourceApple::getBuffer() const
	{
		//return this->sound->getBuffer();
		return (int)avAudioPlayer;
	}
	
	bool SourceApple::isPlaying()
	{
		//NSLog(@"isplaying %@", avAudioPlayerDelegate.playing ? @"da" : @"ne");
		return avAudioPlayerDelegate.playing;
	}

	bool SourceApple::isPaused()
	{
		return (this->paused && !this->isFading());
	}
	
	bool SourceApple::isFading()
	{
		return (this->fadeSpeed != 0.0f);
	}

	bool SourceApple::isFadingIn()
	{
		return (this->fadeSpeed > 0.0f);
	}

	bool SourceApple::isFadingOut()
	{
		return (this->fadeSpeed < 0.0f);
	}
    
    float SourceApple::getDuration()
    {
        return avAudioPlayer.duration;
    }

}