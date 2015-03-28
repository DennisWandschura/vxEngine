#include "RenderStage.h"
#include <vxLib/gl/StateManager.h>
#include <vxLib/gl/gl.h>

RenderStage::RenderStage()
{
}

RenderStage::RenderStage(const RenderStageDescription &desc)
	:m_viewportSize(desc.m_viewportSize),
	m_frameBuffer(desc.m_frameBuffer),
	m_clearBits(desc.m_clearBits),
	m_vao(desc.m_vao),
	m_pipeline(desc.m_pipeline),
	m_drawCommand(desc.m_drawCommandDesc)
{
}

void RenderStage::setState()
{
	vx::gl::StateManager::setViewport(0, 0, m_viewportSize.x, m_viewportSize.y);

	vx::gl::StateManager::bindFrameBuffer(m_frameBuffer);
	glClear(m_clearBits);

	vx::gl::StateManager::bindPipeline(m_pipeline);
	vx::gl::StateManager::bindVertexArray(m_vao);
}

void RenderStage::drawArrays()
{
	setState();

	m_drawCommand.drawArrays();
}

void RenderStage::drawElements()
{
	setState();

	m_drawCommand.drawElements();
}

void RenderStage::multiDrawElementsIndirect()
{
	setState();

	m_drawCommand.multiDrawIndirect();
}