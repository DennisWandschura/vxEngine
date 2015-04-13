#include "Vertex.h"
#include <vxLib\gl\gl.h>
#include <cstring>
#include <vxLib/gl/VertexArray.h>
#include <vxLib/gl/Buffer.h>

namespace detail
{
	void VertexWrapperIbo::create(vx::gl::VertexArray &vao, U32 &ibo, U32 indexCount)
	{
		glCreateBuffers(1, &ibo);
		glNamedBufferStorage(ibo, sizeof(U32) * indexCount, nullptr, GL_MAP_WRITE_BIT);

		vao.bindIndexBuffer(ibo);
	}

	void VertexWrapperIbo::update(const U32 ibo, const U32 *pIndices, U32 indexCount)
	{
		U32 *pIndicesGPU = (U32*)glMapNamedBuffer(ibo, GL_WRITE_ONLY);
		memcpy(pIndicesGPU, pIndices, sizeof(U32) * indexCount);
		glUnmapNamedBuffer(ibo);
	}
}

void VertexPNTUV::create(vx::gl::VertexArray* vao, vx::gl::Buffer* vbo, U32 vertexCount, U32 bindingIndex, U32 &attributeOffset)
{
	*vbo = vx::gl::BufferDescription::createImmutable(vx::gl::BufferType::Array_Buffer, sizeof(value_type) * vertexCount, vx::gl::BufferStorageFlags::Write, nullptr);

	vao->create();

	// position
	vao->enableArrayAttrib(attributeOffset);
	vao->arrayAttribFormatF(attributeOffset, 4, 0, 0);
	vao->arrayAttribBinding(attributeOffset, bindingIndex);
	++attributeOffset;

	// normal
	vao->enableArrayAttrib(attributeOffset);
	vao->arrayAttribFormatF(attributeOffset, 3, 0, sizeof(F32) * 4);
	vao->arrayAttribBinding(attributeOffset, bindingIndex);
	++attributeOffset;

	// tangent
	vao->enableArrayAttrib(attributeOffset);
	vao->arrayAttribFormatF(attributeOffset, 3, 0, sizeof(F32) * 7);
	vao->arrayAttribBinding(attributeOffset, bindingIndex);
	++attributeOffset;

	// uv
	vao->enableArrayAttrib(attributeOffset);
	vao->arrayAttribFormatF(attributeOffset, 2, 0, sizeof(F32) * 10);
	vao->arrayAttribBinding(attributeOffset, bindingIndex);
	++attributeOffset;

	vao->bindVertexBuffer(*vbo, bindingIndex, 0, sizeof(value_type));
}

void VertexPNTUV::update(const U32 vbo, const VertexPNTUV* __restrict pVertices, U32 vertexCount)
{
	VertexPNTUV *pVerticesGPU = (VertexPNTUV*)glMapNamedBuffer(vbo, GL_WRITE_ONLY);
	memcpy(pVerticesGPU, pVertices, sizeof(VertexPNTUV) * vertexCount);
	glUnmapNamedBuffer(vbo);
}

/*



*/

void VertexDrawId::create(vx::gl::VertexArray &vao, U32 &vbo, U32 vertexCount, U32 bindingIndex, U32 &attributeOffset)
{
	glCreateBuffers(1, &vbo);

	glNamedBufferStorage(vbo, sizeof(VertexDrawId) * vertexCount, nullptr, GL_MAP_WRITE_BIT);

	// draw id
	vao.enableArrayAttrib(attributeOffset);
	vao.arrayAttribFormatI(attributeOffset, 1, vx::gl::DataType::Unsigned_Int, 0);
	vao.arrayAttribBinding(attributeOffset, bindingIndex);
	++attributeOffset;

	vao.bindVertexBuffer(vbo, bindingIndex, 0, sizeof(VertexDrawId));
	glVertexArrayBindingDivisor(vao.getId(), bindingIndex, 1);
}

void VertexDrawId::update(const U32 vbo, const VertexDrawId* __restrict pVertices, U32 vertexCount)
{
	VertexDrawId *pVerticesGPU = reinterpret_cast<VertexDrawId*>(glMapNamedBuffer(vbo, GL_WRITE_ONLY));
	memcpy(pVerticesGPU, pVertices, sizeof(VertexDrawId) * vertexCount);
	glUnmapNamedBuffer(vbo);
}