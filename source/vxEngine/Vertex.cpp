#include "Vertex.h"
#include <vxLib\gl\gl.h>
#include <cstring>
#include <vxLib/gl/VertexArray.h>

namespace detail
{
	void VertexWrapperIbo::create(vx::gl::VertexArray &vao, U32 &ibo, U32 indexCount)
	{
#ifdef _VX_GL_45
		glCreateBuffers(1, &ibo);
		glNamedBufferStorage(ibo, sizeof(U32) * indexCount, nullptr, GL_MAP_WRITE_BIT);
#else
		glGenBuffers(1, &ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(U32) * indexCount, nullptr, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
#endif

		vao.bindIndexBuffer(ibo);
	}

	void VertexWrapperIbo::update(const U32 ibo, const U32 *pIndices, U32 indexCount)
	{
#ifdef _VX_GL_45
		U32 *pIndicesGPU = (U32*)glMapNamedBuffer(ibo, GL_WRITE_ONLY);
		memcpy(pIndicesGPU, pIndices, sizeof(U32) * indexCount);
		glUnmapNamedBuffer(ibo);
#else
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

		U32 *pIndicesGPU = (U32*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
		memcpy(pIndicesGPU, pIndices, sizeof(U32) * indexCount);
		glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
#endif
	}
}

void VertexPNTUV::create(vx::gl::VertexArray &vao, U32 &vbo, U32 vertexCount, U32 bindingIndex, U32 &attributeOffset)
{
#ifdef _VX_GL_45
	glCreateBuffers(1, &vbo);

	glNamedBufferStorage(vbo, sizeof(VertexPNTUV) * vertexCount, nullptr, GL_MAP_WRITE_BIT);
#else
	glGenBuffers(1, &vbo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexPNTUV) * vertexCount, nullptr, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
#endif

	// position
	vao.enableArrayAttrib(attributeOffset);
	vao.arrayAttribFormatF(attributeOffset, 4, 0, 0);
	vao.arrayAttribBinding(attributeOffset, bindingIndex);
	++attributeOffset;

	// normal
	vao.enableArrayAttrib(attributeOffset);
	vao.arrayAttribFormatF(attributeOffset, 3, 0, sizeof(F32) * 4);
	vao.arrayAttribBinding(attributeOffset, bindingIndex);
	++attributeOffset;

	// tangent
	vao.enableArrayAttrib(attributeOffset);
	vao.arrayAttribFormatF(attributeOffset, 3, 0, sizeof(F32) * 7);
	vao.arrayAttribBinding(attributeOffset, bindingIndex);
	++attributeOffset;

	// uv
	vao.enableArrayAttrib(attributeOffset);
	vao.arrayAttribFormatF(attributeOffset, 2, 0, sizeof(F32) * 10);
	vao.arrayAttribBinding(attributeOffset, bindingIndex);
	++attributeOffset;

	vao.bindVertexBuffer(vbo, bindingIndex, 0, sizeof(VertexPNTUV));
}

void VertexPNTUV::update(const U32 vbo, const VertexPNTUV* __restrict pVertices, U32 vertexCount)
{
#ifdef _VX_GL_45
	VertexPNTUV *pVerticesGPU = (VertexPNTUV*)glMapNamedBuffer(vbo, GL_WRITE_ONLY);
	memcpy(pVerticesGPU, pVertices, sizeof(VertexPNTUV) * vertexCount);
	glUnmapNamedBuffer(vbo);
#else
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	VertexPNTUV *pVerticesGPU = (VertexPNTUV*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	memcpy(pVerticesGPU, pVertices, sizeof(VertexPNTUV) * vertexCount);
	glUnmapBuffer(GL_ARRAY_BUFFER);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
#endif
}

/*



*/

void VertexDrawId::create(vx::gl::VertexArray &vao, U32 &vbo, U32 vertexCount, U32 bindingIndex, U32 &attributeOffset)
{
#ifdef _VX_GL_45
	glCreateBuffers(1, &vbo);

	glNamedBufferStorage(vbo, sizeof(VertexDrawId) * vertexCount, nullptr, GL_MAP_WRITE_BIT);

	// draw id
	vao.enableArrayAttrib(attributeOffset);
	vao.arrayAttribFormatI(attributeOffset, 1, vx::gl::UNSIGNED_INT, 0);
	vao.arrayAttribBinding(attributeOffset, bindingIndex);
	++attributeOffset;

	vao.bindVertexBuffer(vbo, bindingIndex, 0, sizeof(VertexDrawId));
	glVertexArrayBindingDivisor(vao.getId(), bindingIndex, 1);
#else
	glGenBuffers(1, &vbo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexDrawId) * vertexCount, nullptr, GL_DYNAMIC_DRAW);

	// draw id
	vao.enableArrayAttrib(attributeOffset);
	vao.arrayAttribFormatI(attributeOffset, 1, vx::gl::UNSIGNED_INT, 0);
	vao.arrayAttribBinding(attributeOffset, bindingIndex);
	++attributeOffset;

	vao.bindVertexBuffer(vbo, bindingIndex, 0, sizeof(VertexDrawId));
	vao.arrayBindingDivisor(bindingIndex, 1);
	//glVertexArrayBindingDivisor(vao.getId(), bindingIndex, 1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
#endif
}

void VertexDrawId::update(const U32 vbo, const VertexDrawId* __restrict pVertices, U32 vertexCount)
{
#ifdef _VX_GL_45
	VertexDrawId *pVerticesGPU = reinterpret_cast<VertexDrawId*>(glMapNamedBuffer(vbo, GL_WRITE_ONLY));
	memcpy(pVerticesGPU, pVertices, sizeof(VertexDrawId) * vertexCount);
	glUnmapNamedBuffer(vbo);
#else
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	VertexDrawId *pVerticesGPU = reinterpret_cast<VertexDrawId*>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
	memcpy(pVerticesGPU, pVertices, sizeof(VertexDrawId) * vertexCount);
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
#endif
}