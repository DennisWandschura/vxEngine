#pragma once

#include <vxLib\types.h>
#include <vxLib\gl\ProgramPipeline.h>
#include <vxLib/gl/Buffer.h>
#include <vxLib/gl/VertexArray.h>
#include <vxLib\Container\bitset.h>
#include <vxLib\gl\Framebuffer.h>
#include <vxLib\math\Vector.h>

namespace vx
{
	namespace gl
	{
		class StateManager;
	}
}

// size 28 bytes, align 4
class RenderStageObjects
{
	// 8
	const vx::gl::ProgramPipeline *m_pPipeline;
	// 8
	vx::gl::DrawIndirectBuffer m_drawIndirectBuffer;
	// 8
	vx::gl::VertexArray m_vao;
	// 4
	U32 m_drawCount;

	void drawElements(U32 elementCount);
	void multiDrawIndirect(vx::gl::StateManager &stateManager, U32 indirectCount);

public:
	RenderStageObjects();
	RenderStageObjects(RenderStageObjects &&rhs);
	RenderStageObjects(const vx::gl::ProgramPipeline *pPipeline, const vx::gl::VertexArray &vao, U32 drawCount);
	RenderStageObjects(const vx::gl::ProgramPipeline *pPipeline, const vx::gl::DrawIndirectBuffer &drawIndirectBuffer, const vx::gl::VertexArray &vao, U32 drawCount);

	RenderStageObjects& operator=(RenderStageObjects &&rhs);

	void draw(vx::gl::StateManager &stateManager);

	void setDrawCount(U32 count);

	vx::gl::VertexArray& getVao(){ return m_vao; }
	const vx::gl::VertexArray& getVao() const{ return m_vao; }
};

// size: 48, align 4
class RenderStage
{
	// 28
	RenderStageObjects m_stageObjects;
	// 4
	vx::bitset<64> m_capabilities;
	// 8
	vx::uint2 m_viewportSize;
	// 4
	U32 m_clearBit;
	// 4
	vx::gl::Framebuffer m_frameBuffer;

public:
	RenderStage();
	RenderStage(const vx::gl::ProgramPipeline *pPipeline, const vx::gl::VertexArray &vao, const vx::gl::DrawIndirectBuffer &drawIndirectBuffer, U32 drawCount, const vx::uint2 &viewportSize, U32 clearBit, vx::gl::Framebuffer &&fb);
	RenderStage(const vx::gl::ProgramPipeline *pPipeline, const vx::gl::VertexArray &vao, U32 drawCount, const vx::uint2 &viewportSize, U32 clearBit, vx::gl::Framebuffer &&fb);

	RenderStage& operator=(const RenderStage&) = delete;
	RenderStage& operator=(RenderStage &&rhs);

	void setCapability(vx::gl::Capabilities cap, U8 enabled);

	// count is amound of indices or amound of DrawIndirectCmds
	void draw(vx::gl::StateManager &stateManager);

	void setDrawCount(U32 count);

	vx::gl::VertexArray& getVao(){ return m_stageObjects.getVao(); }
	const vx::gl::VertexArray& getVao() const{ return m_stageObjects.getVao(); }
};