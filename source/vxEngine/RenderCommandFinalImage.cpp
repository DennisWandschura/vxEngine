#include "RenderCommandFinalImage.h"
#include <vxLib/gl/gl.h>
#include <vxLib/gl/StateManager.h>

void RenderCommandFinalImage::render(U32 count)
{
	vx::gl::StateManager::bindFrameBuffer(m_frameBuffer);
	vx::gl::StateManager::disable(vx::gl::Capabilities::Depth_Test);

	vx::gl::StateManager::bindPipeline(m_pipeline);
	vx::gl::StateManager::setViewport(0, 0, m_resolution.x, m_resolution.y);
	vx::gl::StateManager::bindVertexArray(m_vao);

	glDrawArrays(GL_POINTS, 0, count);
}