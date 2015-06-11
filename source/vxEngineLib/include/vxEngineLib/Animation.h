#pragma once

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
		u32 frameRate;

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
		void saveToFile(File* f) const;
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
		void saveToFile(File* f) const;
	};
}