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

struct Light;

namespace vx
{
	class Camera;
}

namespace Gpu
{
	struct LightData;
};

#include "Renderer.h"
#include <vxLib/memory.h>

namespace Graphics
{
	class LightRenderer : public Renderer
	{
		std::unique_ptr<Gpu::LightData[]> m_lights;
		std::unique_ptr<Gpu::LightData[]> m_activeLights;
		std::unique_ptr<std::pair<f32, u32>[]> m_lightDistances;
		u32 m_lightCount;
		u32 m_maxActiveLights;
		u32 m_activeLightCount;

	public:
		LightRenderer();
		~LightRenderer();

		bool initialize(vx::StackAllocator* scratchAllocator, const void* p);
		void shutdown();

		void getCommandList(CommandList* cmdList);

		void clearData();
		void bindBuffers();

		void setLights(const Light* lights, u32 count);

		void cullLights(const vx::Camera &camera);
	};
}
