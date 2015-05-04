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