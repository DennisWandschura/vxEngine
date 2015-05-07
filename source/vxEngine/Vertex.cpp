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
#include "Vertex.h"
#include <vxLib\gl\gl.h>
#include <cstring>
#include <vxLib/gl/VertexArray.h>
#include <vxLib/gl/Buffer.h>

namespace detail
{
	void VertexWrapperIbo::create(vx::gl::VertexArray &vao, u32 &ibo, u32 indexCount)
	{
		glCreateBuffers(1, &ibo);
		glNamedBufferStorage(ibo, sizeof(u32) * indexCount, nullptr, GL_MAP_WRITE_BIT);

		vao.bindIndexBuffer(ibo);
	}

	void VertexWrapperIbo::update(const u32 ibo, const u32 *pIndices, u32 indexCount)
	{
		u32 *pIndicesGPU = (u32*)glMapNamedBuffer(ibo, GL_WRITE_ONLY);
		memcpy(pIndicesGPU, pIndices, sizeof(u32) * indexCount);
		glUnmapNamedBuffer(ibo);
	}
}

void VertexPNTUV::create(vx::gl::VertexArray* vao, vx::gl::Buffer* vbo, u32 vertexCount, u32 bindingIndex, u32 &attributeOffset)
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
	vao->arrayAttribFormatF(attributeOffset, 3, 0, sizeof(f32) * 4);
	vao->arrayAttribBinding(attributeOffset, bindingIndex);
	++attributeOffset;

	// tangent
	vao->enableArrayAttrib(attributeOffset);
	vao->arrayAttribFormatF(attributeOffset, 3, 0, sizeof(f32) * 7);
	vao->arrayAttribBinding(attributeOffset, bindingIndex);
	++attributeOffset;

	// uv
	vao->enableArrayAttrib(attributeOffset);
	vao->arrayAttribFormatF(attributeOffset, 2, 0, sizeof(f32) * 10);
	vao->arrayAttribBinding(attributeOffset, bindingIndex);
	++attributeOffset;

	vao->bindVertexBuffer(*vbo, bindingIndex, 0, sizeof(value_type));
}

void VertexPNTUV::update(const u32 vbo, const VertexPNTUV* __restrict pVertices, u32 vertexCount)
{
	VertexPNTUV *pVerticesGPU = (VertexPNTUV*)glMapNamedBuffer(vbo, GL_WRITE_ONLY);
	memcpy(pVerticesGPU, pVertices, sizeof(VertexPNTUV) * vertexCount);
	glUnmapNamedBuffer(vbo);
}

/*



*/

void VertexDrawId::create(vx::gl::VertexArray &vao, u32 &vbo, u32 vertexCount, u32 bindingIndex, u32 &attributeOffset)
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

void VertexDrawId::update(const u32 vbo, const VertexDrawId* __restrict pVertices, u32 vertexCount)
{
	VertexDrawId *pVerticesGPU = reinterpret_cast<VertexDrawId*>(glMapNamedBuffer(vbo, GL_WRITE_ONLY));
	memcpy(pVerticesGPU, pVertices, sizeof(VertexDrawId) * vertexCount);
	glUnmapNamedBuffer(vbo);
}