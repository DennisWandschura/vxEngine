#pragma once

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

#include <vxEngineLib/Transform.h>
#include <memory>

namespace vx
{
	class File;

	struct AnimationSample
	{
		vx::Transform transform;
		u32 frame;
	};

	struct AnimationLayer
	{
		std::unique_ptr<AnimationSample[]> samples;
		u32 frameCount;
		f32 frameRate;

		AnimationLayer() :samples(), frameCount(0), frameRate(0) {}
		AnimationLayer(const AnimationLayer&) = delete;
		AnimationLayer(AnimationLayer &&rhs) :samples(std::move(rhs.samples)), frameCount(rhs.frameCount), frameRate(rhs.frameRate) {}

		AnimationLayer& operator=(const AnimationLayer&) = delete;

		AnimationLayer& operator=(AnimationLayer &&rhs)
		{
			if (this != &rhs)
			{
				std::swap(samples, rhs.samples);
				std::swap(frameCount, rhs.frameCount);
				std::swap(frameRate, rhs.frameRate);
			}
			return *this;
		}

		const u8* loadFromMemory(const u8 *ptr);
		void saveToFile(File* f, u32* writtenSize) const;
	};

	struct Animation
	{
		std::unique_ptr<AnimationLayer[]> layers;
		u32 layerCount;

		Animation() :layers(), layerCount(0){}
		Animation(const Animation&) = delete;
		Animation(Animation &&rhs) :layers(std::move(rhs.layers)), layerCount(rhs.layerCount){}

		Animation& operator=(const Animation&) = delete;

		Animation& operator=(Animation &&rhs)
		{
			if (this != &rhs)
			{
				std::swap(layers, rhs.layers);
				std::swap(layerCount, rhs.layerCount);
			}
			return *this;
		}

		const u8* loadFromMemory(const u8 *ptr);
		void saveToFile(File* f, u32* writtenSize) const;
	};
}