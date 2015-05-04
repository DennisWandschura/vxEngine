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
#include "ProfilerGraph.h"
#include <algorithm>
#include <vxLib/gl/ShaderManager.h>
#include <vxLib/gl/gl.h>
#include <vxLib/gl/StateManager.h>
#include <Windows.h>
#include <vxLib/gl/ProgramPipeline.h>

I64 ProfilerGraph::s_frequency{ 1 };

const F32 g_timeScale = 10.0f;
const F32 g_graphHeight = 300.0f;

bool ProfilerGraph::initialize(const vx::gl::ShaderManager &shaderManager, F32 targetMs)
{
	glCreateQueries(GL_TIMESTAMP, s_queryCount, m_queryGpuStart);
	glCreateQueries(GL_TIMESTAMP, s_queryCount, m_queryGpuEnd);

	QueryPerformanceFrequency((LARGE_INTEGER*)&s_frequency);

	m_pPipeline = shaderManager.getPipeline("graph.pipe");
	if (!m_pPipeline)
		return false;

	m_position = vx::float2(-s_sampleCount / 2.0f, 10 + (-1080 / 2));
	m_scale = g_graphHeight / (targetMs * g_timeScale * 1000.f);

	const U32 vertexCount = s_sampleCount * 2 + 8;
	vx::float2 vertices[vertexCount];
	vx::uchar4 colors[vertexCount];

	for (auto i = 0; i < s_sampleCount; ++i)
	{
		m_entriesCpu[i].x = i + m_position.x;
		m_entriesCpu[i].y = m_position.y;

		vertices[i] = m_entriesCpu[i];
		colors[i] = vx::uchar4(200, 200, 0, 255);
	}

	for (auto i = 0; i < s_sampleCount; ++i)
	{
		m_entriesGpu[i].x = i + m_position.x;
		m_entriesGpu[i].y = m_position.y;

		vertices[i + s_sampleCount] = m_entriesGpu[i];
		colors[i + s_sampleCount] = vx::uchar4(200, 0, 200, 255);
	}

	vx::uchar4 color = { 0, 0, 255, 255 };

	// right line
	vertices[vertexCount - 8] = vx::float2(m_position.x + s_sampleCount, m_position.y);
	colors[vertexCount - 8] = color;
	vertices[vertexCount - 7] = vx::float2(m_position.x + s_sampleCount, m_position.y + g_graphHeight);
	colors[vertexCount - 7] = color;

	// left line
	vertices[vertexCount - 6] = m_position;
	colors[vertexCount - 6] = color;
	vertices[vertexCount - 5] = vx::float2(m_position.x, m_position.y + g_graphHeight);
	colors[vertexCount - 5] = color;

	// bottom line
	vertices[vertexCount - 4] = m_position;
	colors[vertexCount - 4] = color;
	vertices[vertexCount - 3] = vx::float2(m_position.x + s_sampleCount, m_position.y);
	colors[vertexCount - 3] = color;

	// top line
	vertices[vertexCount - 2] = vx::float2(m_position.x, m_position.y + g_graphHeight);
	colors[vertexCount - 2] = color;
	vertices[vertexCount - 1] = vx::float2(m_position.x + s_sampleCount, m_position.y + g_graphHeight);
	colors[vertexCount - 1] = color;

	vx::gl::BufferDescription vboDesc;
	vboDesc.bufferType = vx::gl::BufferType::Array_Buffer;
#ifdef _VX_GL_45
	vboDesc.flags = vx::gl::BufferStorageFlags::Write | vx::gl::BufferStorageFlags::Dynamic_Storage;
	vboDesc.immutable = 1;
#else
	vboDesc.usage = vx::gl::BufferDataUsage::Dynamic_Draw;
#endif
	vboDesc.size = sizeof(vx::float2) * vertexCount;
	vboDesc.pData = vertices;
	m_vbo.create(vboDesc);

#ifdef _VX_GL_45
	vboDesc.flags = vx::gl::BufferStorageFlags::None;
#else
	vboDesc.usage = vx::gl::BufferDataUsage::Static_Draw;
#endif
	vboDesc.size = sizeof(vx::uchar4) * vertexCount;
	vboDesc.pData = colors;
	m_vboColor.create(vboDesc);

	const int i = 2;
	const int a = i;
	const int b = i + 1;

	const auto indexHalfCount = (s_sampleCount - 1)* 2u;
	const auto indexCount = indexHalfCount + indexHalfCount + 8;
	U16 indices[indexCount];

	for (U32 i = 0, j = 0; i < indexHalfCount; i += 2, ++j)
	{
		indices[i] = j;
		indices[i + 1] = j + 1;
	}

	for (U32 i = 0, j = 0; i < indexHalfCount; i += 2, ++j)
	{
		indices[indexHalfCount + i] = s_sampleCount + j;
		indices[indexHalfCount + i + 1] = s_sampleCount + j + 1;
	}

	indices[indexCount - 8] = vertexCount - 8;
	indices[indexCount - 7] = vertexCount - 7;
	indices[indexCount - 6] = vertexCount - 6;
	indices[indexCount - 5] = vertexCount - 5;
	indices[indexCount - 4] = vertexCount - 4;
	indices[indexCount - 3] = vertexCount - 3;
	indices[indexCount - 2] = vertexCount - 2;
	indices[indexCount - 1] = vertexCount - 1;

	vx::gl::BufferDescription iboDesc;
	iboDesc.bufferType = vx::gl::BufferType::Element_Array_Buffer;
	iboDesc.flags = vx::gl::BufferStorageFlags::None;
	iboDesc.immutable = 1;
	iboDesc.size = sizeof(U16) * indexCount;
	iboDesc.pData = indices;
	m_ibo.create(iboDesc);

	m_vao.create();
	m_vao.bindIndexBuffer(m_ibo);

	// vx::float3 inputPosition;
	m_vao.enableArrayAttrib(0);
	m_vao.arrayAttribFormatF(0, 2, 0, 0);
	m_vao.arrayAttribBinding(0, 0);

	m_vao.enableArrayAttrib(1);
	m_vao.arrayAttribFormatI(1, 1, vx::gl::DataType::Unsigned_Int, 0);
	m_vao.arrayAttribBinding(1, 1);

	m_vao.bindVertexBuffer(m_vbo, 0, 0, sizeof(vx::float2));
	m_vao.bindVertexBuffer(m_vboColor, 1, 0, sizeof(U32));

	m_indexCount = indexCount;

	return true;
}

void ProfilerGraph::startCpu()
{
	QueryPerformanceCounter((LARGE_INTEGER*)&m_currentStart);
}

void ProfilerGraph::endCpu()
{
	LARGE_INTEGER end;
	QueryPerformanceCounter(&end);

	I64 dd = (end.QuadPart - m_currentStart) * 1000000;
	F32 time = dd / s_frequency * 0.001f;

	for (auto i = 0u; i < s_sampleCount - 1; ++i)
	{
		m_entriesCpu[i].y = m_entriesCpu[i + 1].y;
	}

	m_entriesCpu[s_sampleCount - 1].y = time * g_timeScale * m_scale + m_position.y;
}

void ProfilerGraph::startGpu()
{
	glQueryCounter(m_queryGpuStart[m_currentQuery], GL_TIMESTAMP);
}

void ProfilerGraph::endGpu()
{

	glQueryCounter(m_queryGpuEnd[m_currentQuery], GL_TIMESTAMP);

}

void ProfilerGraph::frame(F32 frameTime)
{
	m_currentQuery = (m_currentQuery + 1) % s_queryCount;

	auto getIndex = (m_currentQuery + s_queryCount) % s_queryCount;

	GLint start_ok = 0;
	GLint end_ok = 0;
	glGetQueryObjectiv(m_queryGpuStart[getIndex], GL_QUERY_RESULT_AVAILABLE, &start_ok);
	glGetQueryObjectiv(m_queryGpuEnd[getIndex], GL_QUERY_RESULT_AVAILABLE, &end_ok);
	bool ok = (bool)(start_ok && end_ok);

	if (ok)
	{
		U64 start, end;
		glGetQueryObjectui64v(m_queryGpuStart[getIndex], GL_QUERY_RESULT, &start);
		glGetQueryObjectui64v(m_queryGpuEnd[getIndex], GL_QUERY_RESULT, &end);

		auto dtime = end - start;

		F32 time = dtime * 1.0e-6;

		for (auto i = 0u; i < s_sampleCount - 1; ++i)
		{
			m_entriesGpu[i].y = m_entriesGpu[i + 1].y;
		}

		m_entriesGpu[s_sampleCount - 1].y = time * g_timeScale * m_scale + m_position.y;
	}
}

void ProfilerGraph::update()
{
#ifdef _VX_GL_45
	glNamedBufferSubData(m_vbo.getId(), 0, sizeof(vx::float2) * s_sampleCount, m_entriesCpu);
	glNamedBufferSubData(m_vbo.getId(), sizeof(vx::float2) * s_sampleCount, sizeof(vx::float2) * s_sampleCount, m_entriesGpu);
#else
	m_vbo.bind();

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vx::float2) * s_sampleCount, m_entriesCpu);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vx::float2) * s_sampleCount, sizeof(vx::float2) * s_sampleCount, m_entriesGpu);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
#endif
	//auto p = (vx::float2*)m_vbo.map(vx::gl::Map::Write_Only);
	//memcpy(p, m_entriesCpu, sizeof(vx::float2) * s_sampleCount);
	//memcpy(p + s_sampleCount, m_entriesGpu, sizeof(vx::float2) * s_sampleCount);
	//m_vbo.unmap();
}

void ProfilerGraph::render()
{
	vx::gl::StateManager::bindPipeline(m_pPipeline->getId());
	vx::gl::StateManager::bindVertexArray(m_vao);

	glLineWidth(2.0f);
	//glPointSize(2.0f);
	glDrawElements(GL_LINES, m_indexCount, GL_UNSIGNED_SHORT, 0);
}