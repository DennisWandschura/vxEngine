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
			glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_indirectBuffer);
		}

		if (m_paramBuffer != 0)
		{
			glBindBuffer(GL_PARAMETER_BUFFER_ARB, m_paramBuffer);
		}
	}
}