/*
The MIT License (MIT)

Copyright (c) 2015 Dennis Wandschura

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
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