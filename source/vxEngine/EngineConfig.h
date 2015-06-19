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

struct RenderAspectDescription;
class SystemAspect;

namespace vx
{
	class Window;
	class StackAllocator;
}

#include <vxLib/math/Vector.h>
#include <vxEngineLib/ParserNode.h>

struct EngineConfig
{
	Parser::Node m_root;
	vx::uint2 m_resolution{1920, 1080};
	u32 m_shadowMapResolution{2048};
	f32 m_fov{66.0f};
	f32 m_zNear{0.1f};
	f32 m_zFar{1000.0f};
	u32 m_maxActiveLights{ 5 };
	u32 m_maxShadowCastingLights{ 5 };
	u32 m_maxMeshInstances{150};
	u32 m_threads{1};
	u8 m_voxelGiMode{0};
	bool m_vsync{false};
	bool m_renderDebug{false};

	bool loadFromFile(const char* file);

	RenderAspectDescription getRenderAspectDescription(const vx::Window* window, vx::StackAllocator* allocator) const;
};

struct RendererOptions
{
	u8 m_shadows;
	u8 m_voxelGI;
};

extern EngineConfig g_engineConfig;
extern RendererOptions g_rendererOptions;