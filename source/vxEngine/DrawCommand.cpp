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
#include "DrawCommand.h"
#include <vxLib/gl/gl.h>

namespace Graphics
{
	bool DrawCommandDescription::isValid() const
	{
		bool result = true;

		switch (renderMode)
		{
		case RenderMode::DrawElements:
			break;
		case RenderMode::MultiDrawElementsIndirect:
		{
			if (indirectBuffer == 0)
			{
				result = false;
			}
		}break;
		case RenderMode::DrawArrays:
			break;
		default:
			break;
		}

		return result;
	}

	void DrawCommand::initialize(const DrawCommandDescription &desc)
	{
		m_indirectBuffer = desc.indirectBuffer;
		m_count = desc.count;
		setDataType(desc.dataType);
		setPrimitiveMode(desc.primitiveMode);
		m_renderMode = desc.renderMode;
	}

	void DrawCommand::drawArrays() const
	{
		glDrawArrays(m_primitiveMode, 0, m_count);
	}

	void DrawCommand::drawElements() const
	{
		glDrawElements(m_primitiveMode, m_count, m_dataType, 0);
	}

	void DrawCommand::multiDrawElementsIndirect() const
	{
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_indirectBuffer);
		glMultiDrawElementsIndirect(m_primitiveMode, m_dataType, 0, m_count, sizeof(vx::gl::DrawElementsIndirectCommand));
	}

	void DrawCommand::draw() const
	{
		switch (m_renderMode)
		{
		case RenderMode::DrawElements:
			drawElements();
			break;
		case RenderMode::MultiDrawElementsIndirect:
			multiDrawElementsIndirect();
			break;
		case RenderMode::DrawArrays:
			drawArrays();
			break;
		default:
			break;
		}
	}

	void DrawCommand::setPrimitiveMode(PrimitiveMode mode)
	{
		switch (mode)
		{
		case PrimitiveMode::Points:
			m_primitiveMode = GL_POINTS;
			break;
		case PrimitiveMode::Triangles:
			m_primitiveMode = GL_TRIANGLES;
			break;
		default:
			break;
		}
	}

	void DrawCommand::setRenderMode(RenderMode renderMode)
	{
		m_renderMode = renderMode;
	}

	void DrawCommand::setDataType(vx::gl::DataType dataType)
	{
		switch (dataType)
		{
		case vx::gl::DataType::Byte:
			m_dataType = GL_BYTE;
			break;
		case vx::gl::DataType::Unsigned_Byte:
			m_dataType = GL_UNSIGNED_BYTE;
			break;
		case vx::gl::DataType::Short:
			m_dataType = GL_SHORT;
			break;
		case vx::gl::DataType::Unsigned_Short:
			m_dataType = GL_UNSIGNED_SHORT;
			break;
		case vx::gl::DataType::Int:
			m_dataType = GL_INT;
			break;
		case vx::gl::DataType::Unsigned_Int:
			m_dataType = GL_UNSIGNED_INT;
			break;
		default:
			break;
		}
	}
}