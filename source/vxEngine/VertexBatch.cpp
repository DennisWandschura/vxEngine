#include "VertexBatch.h"
#include <vxLib\gl\gl.h>

MultiDrawIndirect::MultiDrawIndirect()
	:m_drawIndirectBuffer(),
	m_indirectCount()
{
}

void MultiDrawIndirect::draw()
{
	m_drawIndirectBuffer.bind();
	glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0, m_indirectCount, 0);
}

void MultiDrawIndirect::create(const vx::gl::BufferDescription &desc)
{
	m_drawIndirectBuffer.create(desc);
}

void MultiDrawIndirect::destroy()
{
	m_drawIndirectBuffer.destroy();
}