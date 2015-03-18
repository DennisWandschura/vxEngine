#include "RenderStage.h"
#include <vxLib\gl\gl.h>
#include <vxLib\gl\StateManager.h>

RenderStageObjects::RenderStageObjects::RenderStageObjects()
	:m_pPipeline(nullptr),
	m_drawIndirectBuffer(),
	m_vao(),
	m_drawCount(0)
{
}

RenderStageObjects::RenderStageObjects(RenderStageObjects &&rhs)
	:m_pPipeline(rhs.m_pPipeline),
	m_drawIndirectBuffer(std::move(rhs.m_drawIndirectBuffer)),
	m_vao(std::move(rhs.m_vao)),
	m_drawCount(rhs.m_drawCount)
{
}

RenderStageObjects::RenderStageObjects(const vx::gl::ProgramPipeline *pPipeline, const vx::gl::VertexArray &vao, U32 drawCount)
	:m_pPipeline(pPipeline),
	m_drawIndirectBuffer(),
	m_vao(vao),
	m_drawCount(drawCount)
{
}

RenderStageObjects::RenderStageObjects(const vx::gl::ProgramPipeline *pPipeline, const vx::gl::DrawIndirectBuffer &drawIndirectBuffer, const vx::gl::VertexArray &vao, U32 drawCount)
	:m_pPipeline(pPipeline),
	m_drawIndirectBuffer(drawIndirectBuffer),
	m_vao(vao),
	m_drawCount(drawCount)
{
}

RenderStageObjects&  RenderStageObjects::operator=(RenderStageObjects &&rhs)
{
	if (this != &rhs)
	{
		m_pPipeline = rhs.m_pPipeline;
		m_drawIndirectBuffer = std::move(rhs.m_drawIndirectBuffer);
		m_vao = std::move(rhs.m_vao);
		m_drawCount = rhs.m_drawCount;
	}
	return *this;
}

void RenderStageObjects::draw(vx::gl::StateManager &stateManager)
{
	stateManager.bindPipeline(m_pPipeline->getId());

	stateManager.bindVertexArray(m_vao.getId());

	if (m_drawIndirectBuffer.valid())
	{
		multiDrawIndirect(stateManager, m_drawCount);
	}
	else
	{
		drawElements(m_drawCount);
	}
}

void RenderStageObjects::setDrawCount(U32 count)
{
	m_drawCount = count;
}

void RenderStageObjects::drawElements(U32 elementCount)
{
	glDrawElements(GL_TRIANGLES, elementCount, GL_UNSIGNED_INT, 0);
}

void RenderStageObjects::multiDrawIndirect(vx::gl::StateManager &stateManager, U32 indirectCount)
{
	stateManager.bindBuffer(m_drawIndirectBuffer.getTarget(), m_drawIndirectBuffer.getId());
	glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0, indirectCount, 0);
}

RenderStage::RenderStage()
	:m_stageObjects(),
	m_viewportSize(),
	m_clearBit(0),
	m_frameBuffer()

{
}

RenderStage::RenderStage(const vx::gl::ProgramPipeline *pPipeline, const vx::gl::VertexArray &vao, const vx::gl::DrawIndirectBuffer &drawIndirectBuffer,
	U32 drawCount, const vx::uint2 &viewportSize, U32 clearBit, vx::gl::Framebuffer &&fb)
	:m_stageObjects(pPipeline, drawIndirectBuffer, vao, drawCount),
	m_viewportSize(viewportSize),
	m_clearBit(clearBit),
	m_frameBuffer(std::move(fb))
{
}

RenderStage::RenderStage(const vx::gl::ProgramPipeline *pPipeline, const vx::gl::VertexArray &vao, U32 drawCount, const vx::uint2 &viewportSize, U32 clearBit, vx::gl::Framebuffer &&fb)
	:m_stageObjects(pPipeline, vao, drawCount),
	m_viewportSize(viewportSize),
	m_clearBit(clearBit),
	m_frameBuffer(std::move(fb))
{
}

RenderStage& RenderStage::operator = (RenderStage &&rhs)
{
	if (this != &rhs)
	{
		m_stageObjects = std::move(rhs.m_stageObjects);
		m_capabilities = rhs.m_capabilities;
		m_viewportSize = rhs.m_viewportSize;
		m_clearBit = rhs.m_clearBit;
		m_frameBuffer = std::move(rhs.m_frameBuffer);
	}
	return *this;
}

void RenderStage::setCapability(vx::gl::Capabilities cap, U8 enabled)
{
	U8 bitCapability = (U8)cap * 2u;
	m_capabilities.set(bitCapability);
	if (enabled == 0)
		m_capabilities.clear(bitCapability + 1u);
	else
		m_capabilities.set(bitCapability + 1u);
}

void RenderStage::draw( vx::gl::StateManager &stateManager)
{
	for (U32 i = 0; i < vx::gl::g_maxCapabilityCount; i += 2)
	{
		if (m_capabilities[i] != false)
		{
			auto cap = (vx::gl::Capabilities) (i / 2);
			auto index = i + 1;
			if (m_capabilities[index] != false)
				stateManager.enable(cap);
			else
				stateManager.disable(cap);
		}
	}

	stateManager.setViewport(0, 0, m_viewportSize.x, m_viewportSize.y);
	stateManager.bindFrameBuffer(m_frameBuffer.getId());
	glClear(m_clearBit);

	m_stageObjects.draw(stateManager);

	for (U32 i = 0; i < vx::gl::g_maxCapabilityCount; i += 2)
	{
		if (m_capabilities[i] != false)
		{
			auto cap = (vx::gl::Capabilities) (i / 2);

			auto index = i + 1;
			if (m_capabilities[index] != false)
				stateManager.disable(cap);
			else
				stateManager.enable(cap);
		}
	}
}

void RenderStage::setDrawCount(U32 count)
{
	m_stageObjects.setDrawCount(count);
}