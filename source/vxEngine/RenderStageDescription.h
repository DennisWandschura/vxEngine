#pragma once

#include <vxLib/math/Vector.h>
#include "DrawCommandDescription.h"

namespace vx
{
	namespace gl
	{
		class Framebuffer;
		class VertexArray;
		class ProgramPipeline;
	}
}

struct RenderStageDescription
{
	enum class ClearBit : U32 { None = 0, Color = 0x00004000, Depth = 0x00000100, Stencil = 0x00000400, Color_Depth = Color | Depth };

	DrawCommandDescription m_drawCommandDesc;
	vx::uint2 m_viewportSize;
	U32 m_frameBuffer;
	U32 m_clearBits;
	U32 m_vao;
	U32 m_pipeline;

	RenderStageDescription RenderStageDescription::create(const DrawCommandDescription &commandDesc, const vx::uint2 &viewportSize, const vx::gl::Framebuffer &fb,
		ClearBit clearbits, const vx::gl::VertexArray &vao, const vx::gl::ProgramPipeline* pipeline);
};