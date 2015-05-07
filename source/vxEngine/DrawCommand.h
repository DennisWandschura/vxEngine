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

#include <vxLib/gl/Base.h>

namespace Graphics
{
	enum class RenderMode : u8{ DrawArrays, DrawElements, MultiDrawElementsIndirect };
	enum class PrimitiveMode : u8{ Points, Triangles };

	struct DrawCommandDescription
	{
		u32 indirectBuffer{ 0 };
		u32 count{0};
		PrimitiveMode primitiveMode{ PrimitiveMode::Triangles };
		vx::gl::DataType dataType{ vx::gl::DataType::Unsigned_Byte};
		RenderMode renderMode{ RenderMode::DrawArrays };

		bool isValid() const;
	};

	class DrawCommand
	{
		u32 m_indirectBuffer{ 0 };
		u32 m_count{ 0 };
		u16 m_dataType{ 0 };
		u8 m_primitiveMode{ 0 };
		RenderMode m_renderMode{ RenderMode::DrawArrays };

		void drawArrays() const;
		void drawElements() const;
		void multiDrawElementsIndirect() const;

	public:
		DrawCommand() = default;

		void initialize(const DrawCommandDescription &desc);

		void draw() const;

		void setDrawCount(u32 count) { m_count = count; }
		void setPrimitiveMode(PrimitiveMode mode);
		void setRenderMode(RenderMode renderMode);
		void setDataType(vx::gl::DataType dataType);
	};
}