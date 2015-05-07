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
#include <vxLib/gl/StateManager.h>
#include <vxLib/gl/gl.h>

namespace
{
	typedef void(*CapabilityFunctionProc)(vx::gl::Capabilities);

	CapabilityFunctionProc g_capabilityFun[]=
	{
		&vx::gl::StateManager::disable,
		&vx::gl::StateManager::enable
	};
}

namespace Graphics
{
	State::State()
		:m_fbo(0),
		m_vao(0),
		m_pipeline(0),
		m_indirectBuffer(0),
		m_paramBuffer(0),
		m_blendState(0),
		m_depthTestState(1),
		m_polygonOffsetFillState(0)
	{

	}

	State::~State()
	{

	}

	void State::set(const StateDescription &desc)
	{
		m_fbo = desc.fbo;
		m_vao = desc.vao;
		m_pipeline = desc.pipeline;
		m_indirectBuffer = desc.indirectBuffer;
		m_paramBuffer = desc.paramBuffer;
		m_depthTestState = desc.depthState;
		m_blendState = desc.blendState;
		m_polygonOffsetFillState = desc.polygonOffsetFillState;
	}

	void State::update()
	{
		g_capabilityFun[m_depthTestState](vx::gl::Capabilities::Depth_Test);
		g_capabilityFun[m_blendState](vx::gl::Capabilities::Blend);
		g_capabilityFun[m_polygonOffsetFillState](vx::gl::Capabilities::Polygon_Offset_Fill);

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

	void State::compile(std::string* str)
	{
		if (m_blendState == 0)
		{
			*str += std::string("vx::gl::StateManager::disable(vx::gl::Capabilities::Blend);\n");
		}
		else
		{
			*str += std::string("vx::gl::StateManager::enable(vx::gl::Capabilities::Blend);\n");
		}

		if (m_depthTestState == 0)
		{
			*str += std::string("vx::gl::StateManager::disable(vx::gl::Capabilities::Depth_Test);\n");
		}
		else
		{
			*str += std::string("vx::gl::StateManager::enable(vx::gl::Capabilities::Depth_Test);\n");
		}

		*str += std::string("vx::gl::StateManager::bindFrameBuffer(") + std::to_string(m_fbo) + ");\n";
		*str += std::string("vx::gl::StateManager::bindVertexArray(") + std::to_string(m_vao) + std::string(");\n");
		*str += std::string("vx::gl::StateManager::bindPipeline(") + std::to_string(m_pipeline) + std::string(");\n");

		if (m_indirectBuffer != 0)
		{
			*str += std::string("vx::gl::StateManager::bindBuffer(vx::gl::BufferType::Draw_Indirect_Buffer,") + std::to_string(m_indirectBuffer) + std::string(");\n");
		}

		if (m_paramBuffer != 0)
		{
			*str += std::string("vx::gl::StateManager::bindBuffer(vx::gl::BufferType::Parameter_Buffer, ") + std::to_string(m_paramBuffer) + std::string(");\n");
		}
	}
}