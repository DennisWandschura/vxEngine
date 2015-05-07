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

#include "Commands.h"
#include <vxLib/gl/StateManager.h>
#include <vxLib/gl/gl.h>

namespace Graphics
{
	void ViewportCommand::execute(U32* offset)
	{
		vx::gl::StateManager::setViewport(m_offset.x, m_offset.y, m_size.x, m_size.y);

		*offset += sizeof(ViewportCommand);
	}

	void PointSizeCommand::execute(U32* offset)
	{
		glPointSize(m_pointSize);

		*offset += sizeof(PointSizeCommand);
	}

	void DrawArraysIndirectCommand::execute(U32* offset)
	{
		glDrawArraysIndirect(m_mode, (void*)m_offset);

		*offset += sizeof(DrawArraysIndirectCommand);
	}

	void DrawElementsIndirectCommand::execute(U32* offset)
	{
		glDrawElementsIndirect(m_mode, m_type, (void*)m_offset);

		*offset += sizeof(DrawElementsIndirectCommand);
	}

	void MultiDrawElementsIndirectCountCommand::execute(U32* offset)
	{
		glMultiDrawElementsIndirectCountARB(m_mode, m_type, m_indirectOffset, m_parameterBufferOffset, m_maxdrawcount, sizeof(vx::gl::DrawElementsIndirectCommand));

		*offset += sizeof(MultiDrawElementsIndirectCountCommand);
	}

	void MultiDrawArraysIndirectCountCommand::execute(U32* offset)
	{
		glMultiDrawArraysIndirectCountARB(m_mode, m_indirectOffset, m_parameterBufferOffset, m_maxdrawcount, sizeof(vx::gl::DrawArraysIndirectCommand));

		*offset += sizeof(MultiDrawArraysIndirectCountCommand);
	}

	void ClearColorCommand::execute(U32* offset)
	{
		vx::gl::StateManager::setClearColor(m_clearColor.x, m_clearColor.y, m_clearColor.z, m_clearColor.w);

		*offset += sizeof(ClearColorCommand);
	}

	void FramebufferTextureCommand::execute(U32* offset)
	{
		glNamedFramebufferTexture(m_framebufferId, m_attachment, m_texture, m_level);

		*offset += sizeof(FramebufferTextureCommand);
	}

	void PolygonOffsetCommand::execute(U32* offset)
	{
		glPolygonOffset(m_factor, m_units);

		*offset += sizeof(PolygonOffsetCommand);
	}

	void ClearCommand::execute(U32* offset)
	{
		glClear(m_bits);

		*offset += sizeof(ClearCommand);
	}
}