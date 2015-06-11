/*
The MIT License (MIT)

Copyright (c) 2015 Dennis Wandschura

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <vxEngineLib/Animation.h>
#include <vxLib/File/File.h>
#include <vxEngineLib/memcpy.h>

namespace vx
{
	void Animation::saveToFile(File* f) const
	{
		f->write(layerCount);
		for (u32 i = 0; i < layerCount; ++i)
		{
			layers[i].saveToFile(f);
		}
	}

	const u8* Animation::loadFromMemory(const u8 *ptr)
	{
		ptr = vx::read(layerCount, ptr);
		layers = std::make_unique<AnimationLayer[]>(layerCount);
		for (u32 i = 0; i < layerCount; ++i)
		{
			ptr = layers[i].loadFromMemory(ptr);
		}

		return ptr;
	}

	void AnimationLayer::saveToFile(File* f) const
	{
		f->write(frameCount);
		f->write(frameRate);

		for (u32 i = 0; i < frameCount; ++i)
		{
			f->write(samples[i]);
		}
	}

	const u8* AnimationLayer::loadFromMemory(const u8 *ptr)
	{
		ptr = vx::read(frameCount, ptr);
		ptr = vx::read(frameRate, ptr);

		samples = std::make_unique<AnimationSample[]>(frameCount);
		return vx::read(samples.get(), ptr, frameCount);
	}
}