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

//#include "Drawable.h"
#include <vxLib\gl\Base.h>
#include "Vertex.h"
#include <vxLib/gl/Buffer.h>

class MultiDrawIndirect
{

	vx::gl::Buffer m_drawIndirectBuffer;
	U32 m_indirectCount;

public:
	MultiDrawIndirect();

	void create(const vx::gl::BufferDescription &desc);
	void destroy();

	void draw();
};

template<U32 Count, typename ...Args>
class VertexContainer;

template< typename VertexType>
class VertexContainer < 1, VertexType >
{
	using MyWrapper = VertexWrapper < 1, VertexType > ;
	using value_type1 = typename MyWrapper::value_type1;

	U32 m_vertexBuffer[1];
	U32 m_indexBuffer;
	U32 m_indexCount;
	U32 m_vertexCount[1];
	std::unique_ptr<value_type1[]> m_pVertices1;
	std::unique_ptr<U32[]> m_pIndices;

public:
	void create(vx::gl::VertexArray &vao, std::unique_ptr<value_type1[]> &&vertices1, U32 vertexCount1, std::unique_ptr<U32[]>&&indices, U32 indexCount)
	{
		m_vertexCount[0] = vertexCount1;

		MyWrapper::create(vao, m_vertexBuffer, m_vertexCount, m_indexBuffer, indexCount);

		m_pVertices1 = std::move(vertices1);
		m_pIndices = std::move(indices);

		m_indexCount = indexCount;
	}

	void update()
	{
		const void *pVertices[1];
		pVertices[0] = m_pVertices1.get();

		MyWrapper::update(m_vertexBuffer, m_indexBuffer, pVertices, m_vertexCount, m_pIndices.get(), m_indexCount);
	}
};

template< typename VertexType1, typename VertexType2>
class VertexContainer < 2, VertexType1, VertexType2 >
{
	using MyWrapper = VertexWrapper < 2, VertexType1, VertexType2 >;
	using value_type1 = typename MyWrapper::value_type1;
	using value_type2 = typename MyWrapper::value_type2;

	U32 m_vertexBuffer[2];
	U32 m_indexBuffer;
	U32 m_indexCount;
	U32 m_vertexCount[2];
	std::unique_ptr<value_type1[]> m_pVertices1;
	std::unique_ptr<value_type2[]> m_pVertices2;
	std::unique_ptr<U32[]> m_pIndices;

public:
	void create(vx::gl::VertexArray &vao, std::unique_ptr<value_type1[]> &&vertices1, U32 vertexCount1,
		std::unique_ptr<value_type2[]> &&vertices2, U32 vertexCount2, std::unique_ptr<U32[]>&&indices, U32 indexCount)
	{
		m_vertexCount[0] = vertexCount1;
		m_vertexCount[1] = vertexCount2;

		MyWrapper::create(vao, m_vertexBuffer, m_vertexCount, m_indexBuffer, indexCount);

		m_pVertices1 = std::move(vertices1);
		m_pVertices2 = std::move(vertices2);
		m_pIndices = std::move(indices);
		
		m_indexCount = indexCount;
	}

	void update()
	{
		const void *pVertices[2];
		pVertices[0] = m_pVertices1.get();
		pVertices[1] = m_pVertices2.get();

		MyWrapper::update(m_vertexBuffer, m_indexBuffer, pVertices, m_vertexCount, m_pIndices.get(), m_indexCount);
	}
};

template<U32 VBO_count, typename ...VertexTypes>
class VertexBatch;

template<typename VertexType>
class VertexBatch<1, VertexType> : public MultiDrawIndirect
{
	typedef VertexContainer<1, VertexType> MyVertexContainer;
	
	MyVertexContainer m_vertexContainer;

	void create()
	{

	}

public:
	void create(U32 vertexCount, U32 indexCount)
	{
		m_vao.create();
	//	m_vertexContainer.create(m_vao, vertexCount, indexCount);
		MultiDrawIndirect::create();
	}

	void destroy()
	{
		m_vao.destroy();
	}

	void update()
	{
		m_vertexContainer.update();
	}

	void draw()
	{
		MultiDrawIndirect::draw();
	}

	MyVertexContainer& getVertexContainer() { return m_vertexContainer; }
	const MyVertexContainer& getVertexContainer() const { return m_vertexContainer; }
};