#include "RenderStageDescription.h"
#include <vxLib/gl/Framebuffer.h>
#include <vxLib/gl/VertexArray.h>
#include <vxLib/gl/ProgramPipeline.h>

RenderStageDescription RenderStageDescription::create(const DrawCommandDescription &commandDesc, const vx::uint2 &viewportSize, const vx::gl::Framebuffer &fb,
	ClearBit clearbits, const vx::gl::VertexArray &vao, const vx::gl::ProgramPipeline* pipeline)
{
	RenderStageDescription result;

	result.m_drawCommandDesc = commandDesc;
	result.m_viewportSize = viewportSize;
	result.m_frameBuffer = fb.getId();
	result.m_clearBits = (U32)clearbits;
	result.m_vao = vao.getId();
	result.m_pipeline = pipeline->getId();

	return result;
}