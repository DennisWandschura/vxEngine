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
#pragma once

struct RenderAspectDescription;
class SystemAspect;

namespace vx
{
	class Window;
	class StackAllocator;
}

#include <vxLib/math/Vector.h>

class EngineConfig
{
	vx::uint2 m_resolution{1920, 1080};
	U32 m_shadowMapResolution{2048};
	F32 m_fov{66.0f};
	F32 m_zNear{0.1f};
	F32 m_zFar{1000.0f};
	U8 m_voxelGiMode{0};
	bool m_vsync{false};
	bool m_renderDebug{false};

public:
	//void loadFromYAML(const char* file);
	bool loadFromFile(const char* file);

	const vx::uint2& getResolution() const { return m_resolution; }
	bool isVSync() const { return m_vsync; }
	bool isRenderDebug() const { return m_renderDebug; }

	RenderAspectDescription getRenderAspectDescription(const vx::Window* window, vx::StackAllocator* allocator) const;
};