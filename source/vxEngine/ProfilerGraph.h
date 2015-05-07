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

namespace vx
{
	namespace gl
	{
		class ProgramPipeline;
		class StateManager;
		class ShaderManager;
	}
}

#include <vxLib/math/Vector.h>
#include <vxLib/gl/Buffer.h>
#include <vxLib/gl/VertexArray.h>

class ProfilerGraph
{
	static const u16 s_sampleCount = 500u;
	static s64 s_frequency;
	static const u8 s_queryCount = 3;

	s64 m_currentStart{0};
	u32 m_queryGpuStart[s_queryCount];
	u32 m_queryGpuEnd[s_queryCount];
	u32 m_currentQuery{0};
	f32 m_scale{ 1.0f };
	vx::float2 m_position{ 0, 0 };
	vx::float2 m_entriesCpu[s_sampleCount];
	vx::float2 m_entriesGpu[s_sampleCount];
	const vx::gl::ProgramPipeline *m_pPipeline{ nullptr };
	vx::gl::VertexArray m_vao;
	u32 m_indexCount{0};
	vx::gl::Buffer m_vbo;
	vx::gl::Buffer m_ibo;
	vx::gl::Buffer m_vboColor;

public:
	ProfilerGraph() = default;
	bool initialize(const vx::gl::ShaderManager &shaderManager, f32 targetMs);

	void startCpu();
	void endCpu();

	void startGpu();
	void endGpu();

	// call at begin of frame
	void frame(f32 frameTime);
	// updates buffers
	void update();
	void render();
};