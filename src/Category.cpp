/// @file
/// @version 3.4
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/hexception.h>
#include <hltypes/hstring.h>

#include "AudioManager.h"
#include "Category.h"

namespace xal
{
	Category::Category(chstr name, BufferMode bufferMode, SourceMode sourceMode) : gain(1.0f)
	{
		this->name = name;
		this->bufferMode = bufferMode;
		this->sourceMode = sourceMode;
	}

	Category::~Category()
	{
	}

	void Category::setGain(float value)
	{
		hmutex::ScopeLock lock(&xal::manager->mutex);
		this->gain = value;
		xal::manager->_setGlobalGain(xal::manager->globalGain); // updates all Player instances with the new category gain
	}

	bool Category::isStreamed()
	{
		return (this->bufferMode == STREAMED);
	}

	bool Category::isMemoryManaged()
	{
		return (this->bufferMode == MANAGED);
	}

}
