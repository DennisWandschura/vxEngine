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

	static void create(vx::gl::VertexArray* vao, vx::gl::Buffer* vbo, U32 vertexCount, U32 bindingIndex, U32 &attributeOffset);
	static void update(const U32 vbo, const VertexPNTUV* __restrict pVertices, U32 vertexCount);
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
	U32 drawId;

	static void create(vx::gl::VertexArray &vao, U32 &vbo, U32 vertexCount, U32 bindingIndex, U32 &attributeOffset);
	static void update(const U32 vbo, const VertexDrawId* __restrict pVertices, U32 vertexCount);
};

namespace detail
{
	struct VertexWrapperIbo
	{
		static void create(vx::gl::VertexArray &vao, U32 &ibo, U32 indexCount);
		static void update(const U32 ibo, const U32 *pIndices, U32 indexCount);
	};
}

template<U32 Count, typename ...VertexTypes>
struct VertexWrapper;

template<typename VertexType>
struct VertexWrapper < 1, VertexType >
{
	using value_type1 = VertexType;

	static void create(vx::gl::VertexArray &vao, U32(&vbo)[1], const U32(&vertexCount)[1], U32 &ibo, U32 indexCount)
	{
		auto attributeOffset = 0u;
		value_type1::create(vao, vbo[0], vertexCount[0], 0, attributeOffset);
		detail::VertexWrapperIbo::create(vao, ibo, indexCount);
	}

	static void update(const U32(&vbo)[1], const U32 ibo, const void* __restrict pVertices[1], const U32(&vertexCount)[1], const U32* __restrict pIndices, U32 indexCount)
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

	static void create(vx::gl::VertexArray &vao, U32(&vbo)[2], const U32(&vertexCount)[2], U32 &ibo, U32 indexCount)
	{
		auto attributeOffset = 0u;
		value_type1::create(vao, vbo[0], vertexCount[0], 0, attributeOffset);
		value_type2::create(vao, vbo[1], vertexCount[1], 1, attributeOffset);
		detail::VertexWrapperIbo::create(vao, ibo, indexCount);
	}

	static void update(const U32(&vbo)[2], const U32 ibo, const void* __restrict pVertices[2], const U32(&vertexCount)[2], const U32* __restrict pIndices, U32 indexCount)
	{
		value_type1::update(vbo[0], (value_type1*)pVertices[0], vertexCount[0]);
		value_type2::update(vbo[1], (value_type2*)pVertices[1], vertexCount[1]);
		detail::VertexWrapperIbo::update(ibo, pIndices, indexCount);
	}
};