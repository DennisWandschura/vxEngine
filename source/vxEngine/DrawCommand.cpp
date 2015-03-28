#include "DrawCommand.h"
#include <vxLib/gl/gl.h>

DrawCommand::DrawCommand(const DrawCommandDescription &desc)
	:m_indirectBuffer(desc.m_indirectBuffer),
	m_count(),
	m_dataType(vx::gl::detail::getDataType(desc.m_dataType)),
	m_mode(getMode(desc.m_mode))
{
}

U32 DrawCommand::getMode(DrawCommandDescription::Mode mode)
{
	U32 result = 0;

	switch (mode)
	{
	case DrawCommandDescription::Mode::TRIANGLES:
		result = GL_TRIANGLES;
		break;
	case DrawCommandDescription::Mode::POINTS:
		result = GL_POINTS;
		break;
	default:
		break;
	}

	return result;
}

void DrawCommand::drawArrays()
{
	glDrawArrays(m_mode, 0, m_count);
}

void DrawCommand::drawElements()
{
	glDrawElements(m_mode, m_count, m_dataType, 0);
}

void DrawCommand::multiDrawIndirect()
{
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_indirectBuffer);
	glMultiDrawElementsIndirect(m_mode, m_dataType, 0, m_count, 0);
}