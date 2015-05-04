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

namespace Graphics
{
	State::State()
		:m_fbo(0),
		m_vao(0),
		m_pipeline(0),
		m_indirectBuffer(0),
		m_paramBuffer(0),
		m_blendState(0),
		m_depthTestState(1)
	{

	}

	State::~State()
	{

	}

	void State::set(U32 fbo, U32 vao, U32 pipeline, U32 indirectBuffer, U32 paramBuffer)
	{
		m_fbo = fbo;
		m_vao = vao;
		m_pipeline = pipeline;
		m_indirectBuffer = indirectBuffer;
		m_paramBuffer = paramBuffer;
	}

	void State::setDepthTest(bool b)
	{
		m_depthTestState = b;
	}

	void State::setBlendState(bool b)
	{
		m_blendState = b;
	}

	void State::update()
	{
		if (m_blendState == 0)
		{
			vx::gl::StateManager::disable(vx::gl::Capabilities::Blend);
		}
		else
		{
			vx::gl::StateManager::enable(vx::gl::Capabilities::Blend);
		}

		if (m_depthTestState == 0)
		{
			vx::gl::StateManager::disable(vx::gl::Capabilities::Depth_Test);
		}
		else
		{
			vx::gl::StateManager::enable(vx::gl::Capabilities::Depth_Test);
		}

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