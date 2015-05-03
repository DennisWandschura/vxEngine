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
			//*str += std::string("vx::gl::StateManager::disable(vx::gl::Capabilities::Blend);\n");
			*str += std::string("glDisable(GL_BLEND);\n");
		}
		else
		{
			//*str += std::string("vx::gl::StateManager::enable(vx::gl::Capabilities::Blend);\n");
			*str += std::string("glEnable(GL_BLEND);\n");
		}

		if (m_depthTestState == 0)
		{
		//	*str += std::string("vx::gl::StateManager::disable(vx::gl::Capabilities::Depth_Test);\n");
			*str += std::string("glDisable(GL_DEPTH_TEST);\n");
		}
		else
		{
			//*str += std::string("vx::gl::StateManager::enable(vx::gl::Capabilities::Depth_Test);\n");
			*str += std::string("glEnable(GL_DEPTH_TEST);\n");
		}

		//*str += std::string("vx::gl::StateManager::bindFrameBuffer(") + std::to_string(m_fbo) + ");\n";
		*str += std::string("glBindFramebuffer(GL_FRAMEBUFFER, ") + std::to_string(m_fbo) + std::string(");\n");
		//*str += std::string("vx::gl::StateManager::bindVertexArray(") + std::to_string(m_vao) + std::string(");\n");
		*str += std::string("glBindVertexArray(") + std::to_string(m_vao) + std::string(");\n");
		//*str += std::string("vx::gl::StateManager::bindPipeline(") + std::to_string(m_pipeline) + std::string(");\n");
		*str += std::string("glBindProgramPipeline(") + std::to_string(m_pipeline) + std::string(");\n");

		if (m_indirectBuffer != 0)
		{
			//*str += std::string("vx::gl::StateManager::bindBuffer(vx::gl::BufferType::Draw_Indirect_Buffer,") + std::to_string(m_indirectBuffer) + std::string(");\n");
			*str += "glBindBuffer(GL_DRAW_INDIRECT_BUFFER, " + std::to_string(m_indirectBuffer) + ");\n";
		}

		if (m_paramBuffer != 0)
		{
			//*str += std::string("vx::gl::StateManager::bindBuffer(vx::gl::BufferType::Parameter_Buffer, ") + std::to_string(m_paramBuffer) + std::string(");\n");
			*str += "glBindBuffer(GL_PARAMETER_BUFFER_ARB, " + std::to_string(m_paramBuffer) + ");\n";
		}
	}
}