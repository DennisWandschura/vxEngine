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
		class Buffer;
	}
}

#include <vxLib\math\Vector.h>

struct VertexPNTUV
{
	typedef VertexPNTUV value_type;

	vx::float4 position;
	vx::float3 normal;
	vx::float3 tangent;
	vx::float2 uv;

	static void create(vx::gl::VertexArray* vao, vx::gl::Buffer* vbo, u32 vertexCount, u32 bindingIndex, u32 &attributeOffset);
	static void update(const u32 vbo, const VertexPNTUV* __restrict pVertices, u32 vertexCount);
};

struct VertexPNTBUV
{
	vx::float3 position;
	vx::float3 normal;
	vx::float3 tangent;
	vx::float3 bitangent;
	vx::float2 uv;
};

struct VertexDrawId
{
	u32 drawId;

	static void create(vx::gl::VertexArray &vao, u32 &vbo, u32 vertexCount, u32 bindingIndex, u32 &attributeOffset);
	static void update(const u32 vbo, const VertexDrawId* __restrict pVertices, u32 vertexCount);
};

namespace detail
{
	struct VertexWrapperIbo
	{
		static void create(vx::gl::VertexArray &vao, u32 &ibo, u32 indexCount);
		static void update(const u32 ibo, const u32 *pIndices, u32 indexCount);
	};
}

template<u32 Count, typename ...VertexTypes>
struct VertexWrapper;

template<typename VertexType>
struct VertexWrapper < 1, VertexType >
{
	using value_type1 = VertexType;

	static void create(vx::gl::VertexArray &vao, u32(&vbo)[1], const u32(&vertexCount)[1], u32 &ibo, u32 indexCount)
	{
		auto attributeOffset = 0u;
		value_type1::create(vao, vbo[0], vertexCount[0], 0, attributeOffset);
		detail::VertexWrapperIbo::create(vao, ibo, indexCount);
	}

	static void update(const u32(&vbo)[1], const u32 ibo, const void* __restrict pVertices[1], const u32(&vertexCount)[1], const u32* __restrict pIndices, u32 indexCount)
	{
		value_type1::update(vbo[0], (value_type1*)pVertices[0], vertexCount[0]);
		detail::VertexWrapperIbo::update(ibo, pIndices, indexCount);
	}
};

template<typename VertexType1, typename VertexType2>
struct VertexWrapper < 2, VertexType1, VertexType2 >
{
	using value_type1 = VertexType1;
	using value_type2 = VertexType2;

	static void create(vx::gl::VertexArray &vao, u32(&vbo)[2], const u32(&vertexCount)[2], u32 &ibo, u32 indexCount)
	{
		auto attributeOffset = 0u;
		value_type1::create(vao, vbo[0], vertexCount[0], 0, attributeOffset);
		value_type2::create(vao, vbo[1], vertexCount[1], 1, attributeOffset);
		detail::VertexWrapperIbo::create(vao, ibo, indexCount);
	}

	static void update(const u32(&vbo)[2], const u32 ibo, const void* __restrict pVertices[2], const u32(&vertexCount)[2], const u32* __restrict pIndices, u32 indexCount)
	{
		value_type1::update(vbo[0], (value_type1*)pVertices[0], vertexCount[0]);
		value_type2::update(vbo[1], (value_type2*)pVertices[1], vertexCount[1]);
		detail::VertexWrapperIbo::update(ibo, pIndices, indexCount);
	}
};