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
/// Provides an interface for audio sources.

#ifndef XAL_SOURCE_H
#define XAL_SOURCE_H

#include <hltypes/hltypesUtil.h>
#include <hltypes/hstream.h>
#include <hltypes/hstring.h>

#include "AudioManager.h"
#include "xalExport.h"

namespace xal
{
	class Category;

	/// @brief Represents an audio data source.
	class xalExport Source
	{
	public:
		/// @brief Constructor.
		/// @param[in] filename Filename of the Source.
		/// @param[in] sourceMode The Source's behavior mode.
		/// @param[in] bufferMode The Buffer's behavior mode.
		Source(chstr filename, SourceMode sourceMode, BufferMode bufferMode);
		/// @brief Destructor.
		virtual ~Source();

		HL_DEFINE_GET(hstr, filename, Filename);
		HL_DEFINE_GET(SourceMode, sourceMode, SourceMode);
		HL_DEFINE_GET(BufferMode, bufferMode, BufferMode);
		HL_DEFINE_IS(streamOpen, Open);
		HL_DEFINE_GET(int, size, Size);
		HL_DEFINE_GET(int, channels, Channels);
		HL_DEFINE_GET(int, samplingRate, SamplingRate);
		HL_DEFINE_GET(int, bitsPerSample, BitsPerSample);
		HL_DEFINE_GET(float, duration, Duration);
		int getRamSize();

		/// @brief Opens the Source for reading.
		/// @return True if Source was opened successfully.
		virtual bool open();
		/// @brief Closes the Source.
		virtual void close();
		/// @brief Rewinds the Source's audio data to the beginning.
		virtual void rewind();
		/// @brief Loads all audio data.
		/// @param[out] output Data stream where all data will be stored.
		/// @return True if data was successfully read.
		virtual bool load(hstream& output);
		/// @brief Loads a chunk of audio data.
		/// @param[out] output Data stream where all data will be stored.
		/// @param[out] size Maximum byte-size of data to read.
		/// @return Number of bytes read.
		virtual int loadChunk(hstream& output, int size = STREAM_BUFFER_SIZE);
		
	protected:
		/// @brief Filename of the Source.
		hstr filename;
		/// @brief How to handle the Source.
		SourceMode sourceMode;
		/// @brief How to handle the overlying Buffer.
		BufferMode bufferMode;
		/// @brief Whether the underlying stream is open.
		bool streamOpen;
		/// @brief Byte-size of the audio data.
		int size;
		/// @brief Number of channels in the audio data.
		int channels;
		/// @brief Sampling rate of the audio data.
		int samplingRate;
		/// @brief Number of bits per sample in the audio data.
		int bitsPerSample;
		/// @brief Length of the audio data in seconds.
		float duration;
		/// @brief The underlying audio data.
		hsbase* stream;

	};

}

#endif
