#pragma once

namespace vx
{
	namespace gl
	{
		class VertexArray;
		class ProgramPipeline;
	}
}

#include <vxLib/math/Vector.h>

class RenderCommand
{
protected:
	enum Capability : U8
	{
		Depth_Test = 1 << 0,
		Blend = 1 << 1
	};

	U32 m_vao{ 0 };
	U32 m_pipeline{ 0 };
	U32 m_frameBuffer{ 0 };
	vx::uint2 m_resolution;
	U32 m_capability{0};

public:
	void initialize(const vx::gl::VertexArray &vao, const vx::gl::ProgramPipeline &pipeline, const vx::uint2 &resolution);

	virtual void render(U32 count) = 0;
};