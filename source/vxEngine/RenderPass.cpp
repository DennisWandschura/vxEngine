#include "RenderPass.h"
#include <vxLib/gl/StateManager.h>
#include <vxLib/gl/gl.h>

namespace Graphics
{
	void RenderPass::initialize(const RenderPassDescription &desc)
	{
		m_drawCommand.initialize(desc.drawCmdDesc);
		m_resolution = desc.resolution;
		m_vao = desc.vao;
		m_fbo = desc.fbo;
		m_clearBits = desc.clearBits;
		m_pipeline = desc.pipeline;
		m_barrierBits = desc.barrierBits;
	}

	void RenderPass::draw() const
	{
		if (m_barrierBits != 0)
		{
			glMemoryBarrier(m_barrierBits);
		}

		vx::gl::StateManager::setViewport(0, 0, m_resolution.x, m_resolution.y);
		vx::gl::StateManager::bindFrameBuffer(m_fbo);
		vx::gl::StateManager::bindVertexArray(m_vao);
		vx::gl::StateManager::bindPipeline(m_pipeline);

		if (m_clearBits != 0)
		{
			glClear(m_clearBits);
		}

		m_drawCommand.draw();
	}

	void RenderPass::setDrawCount(U32 count)
	{
		m_drawCommand.setDrawCount(count);
	}
}