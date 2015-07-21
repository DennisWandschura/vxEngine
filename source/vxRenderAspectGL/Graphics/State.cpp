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
#include "State.h"
#include <vxGL/StateManager.h>
#include <vxGL/gl.h>

namespace
{
	typedef void(*CapabilityFunctionProc)(vx::gl::Capabilities);

	CapabilityFunctionProc g_capabilityFun[] =
	{
		&vx::gl::StateManager::disable,
		&vx::gl::StateManager::enable
	};
}

namespace Graphics
{
	enum State::GlState : u8
	{
		Blend,
		Depth,
		PolygonOffsetFill,
		CullFace
	};

	State::State()
		:m_fbo(0),
		m_vao(0),
		m_pipeline(0),
		m_indirectBuffer(0),
		m_paramBuffer(0),
		m_state(0 << GlState::Blend | 1 << GlState::Depth | 0 << GlState::PolygonOffsetFill | 1 << GlState::CullFace),
		m_colorMask(1 << 0 | 1 << 1 | 1 << 2 | 1 << 3 | 1 << 4)
		/*m_blendState(0),
		m_depthTestState(1),
		m_polygonOffsetFillState(0),
		m_cullFace(1)*/
	{

	}

	State::~State()
	{

	}

	void State::set(const StateDescription &desc)
	{
		m_fbo = desc.m_fbo;
		m_vao = desc.m_vao;
		m_pipeline = desc.m_pipeline;
		m_indirectBuffer = desc.m_indirectBuffer;
		m_paramBuffer = desc.m_paramBuffer;
		/*m_depthTestState = desc.m_depthState;
		m_blendState = desc.m_blendState;
		m_polygonOffsetFillState = desc.m_polygonOffsetFillState;
		m_cullFace = desc.m_cullface;*/
		m_state = (desc.m_blendState << GlState::Blend) | (desc.m_depthState << GlState::Depth) | (desc.m_polygonOffsetFillState << GlState::PolygonOffsetFill) | (desc.m_cullface << GlState::CullFace);

		m_colorMask = (desc.m_colorMask.x << 0) | (desc.m_colorMask.y << 1) | (desc.m_colorMask.z << 2) | (desc.m_colorMask.w << 3) | (desc.m_depthMask << 4);
	}

	void State::update() const
	{
		auto blendState = (m_state >> GlState::Blend) & 0x1;
		auto depthState = (m_state >> GlState::Depth) & 0x1;
		auto polygonOffsetState = (m_state >> GlState::PolygonOffsetFill) & 0x1;
		auto cullState = (m_state >> GlState::CullFace) & 0x1;

		g_capabilityFun[blendState](vx::gl::Capabilities::Blend);
		g_capabilityFun[depthState](vx::gl::Capabilities::Depth_Test);
		g_capabilityFun[polygonOffsetState](vx::gl::Capabilities::Polygon_Offset_Fill);
		g_capabilityFun[cullState](vx::gl::Capabilities::Cull_Face);

		const auto mm = 1 << 0 | 1 << 1 | 1 << 2 | 1 << 3;
		auto colorMask = m_colorMask & 0xf;
		//vx::gl::StateManager::setColorMask(colorMask);

		u8 depthMask = (m_colorMask >> 4) & 0x1;
		//vx::gl::StateManager::setDepthMask(depthMask);

		vx::gl::StateManager::bindFrameBuffer(m_fbo);
		vx::gl::StateManager::bindVertexArray(m_vao);
		vx::gl::StateManager::bindPipeline(m_pipeline);

		if (m_indirectBuffer != 0)
		{
			vx::gl::StateManager::bindBuffer(vx::gl::BufferType::Draw_Indirect_Buffer, m_indirectBuffer);
		}

		if (m_paramBuffer != 0)
		{
			vx::gl::StateManager::bindBuffer(vx::gl::BufferType::Parameter_Buffer, m_paramBuffer);
		}
	}

	bool State::isValid() const
	{
		bool isValid = (m_vao != 0) && (m_pipeline != 0);

		return isValid;
	}
}