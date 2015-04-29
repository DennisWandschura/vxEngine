#pragma once

#include "DrawCommand.h"
#include <vxLib/gl/Base.h>
#include <vxLib/math/Vector.h>

namespace Graphics
{
	struct RenderPassDescription
	{
		DrawCommandDescription drawCmdDesc;
		vx::uint2 resolution{ 0, 0 };
		U32 vao{ 0 };
		U32 fbo{ 0 };
		U32 clearBits{ 0 };
		U32 pipeline{ 0 };
		U32 barrierBits{ 0 };
	};

	class RenderPass
	{
		DrawCommand m_drawCommand;
		vx::uint2 m_resolution{ 0, 0 };
		U32 m_vao{ 0 };
		U32 m_fbo{ 0 };
		U32 m_clearBits{ 0 };
		U32 m_pipeline{ 0 };
		U32 m_barrierBits{ 0 };

	public:
		void initialize(const RenderPassDescription &desc);

		void draw() const;

		void setDrawCount(U32 count);
	};
}