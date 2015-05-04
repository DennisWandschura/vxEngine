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
#include "GpuProfiler.h"
#include "Font.h"
#include <vxLib/gl/ProgramPipeline.h>
#include <vxLib/gl/StateManager.h>
#include <vxLib/gl/gl.h>
#include <Windows.h>

I64 GpuProfiler::s_cpuFrequency{ 1 };
vx::float2 GpuProfiler::s_position{};

namespace
{
	inline void	incrementCycle(U8* pval, U8 array_size)
	{
		I32 val = *pval;
		++val;

		if (val >= (I32)(array_size))
			val = 0;

		*pval = val;
	}

	inline void	incrementCycle(I32* pval, size_t array_size)
	{
		I32 val = *pval;
		++val;

		if (val >= (I32)(array_size))
			val = 0;

		*pval = val;
	}

	inline void	incrementCycle(I64* pval, size_t array_size)
	{
		I64 val = *pval;
		++val;

		if (val >= (I64)(array_size))
			val = 0;

		*pval = val;
	}

	inline void	incrementCycle(U32* pval, size_t array_size)
	{
		I64 val = *pval;
		++val;

		if (val >= (I64)(array_size))
			val = 0;

		*pval = val;
	}

	inline void	decrementCycle(U32* pval, size_t array_size)
	{
		I64 val = *pval;
		--val;

		if (val < 0)
			val = (I64)(array_size - 1);

		*pval = val;
	}

	inline void	decrementCycle(I64* pval, size_t array_size)
	{
		I64 val = *pval;
		--val;

		if (val < 0)
			val = (I64)(array_size - 1);

		*pval = val;
	}
}

GpuProfiler::GpuProfiler()
{
}

GpuProfiler::~GpuProfiler()
{
}

bool GpuProfiler::initialize(const Font* pFont, const vx::gl::ProgramPipeline* pPipeline, U32 textureIndex, const vx::uint2 windowResolution, vx::StackAllocator* pAllocator)
{
	LARGE_INTEGER frq;
	QueryPerformanceFrequency(&frq);
	s_cpuFrequency = frq.QuadPart;

	m_pPipeline = pPipeline;
	m_pFont = pFont;
	m_textureIndex = textureIndex;

	s_position = vx::float2(windowResolution) / 2.0f;
	s_position.x = -s_position.x + 10.0f;
	s_position.y -= 20.0f;

	m_pVertices = std::make_unique<Vertex[]>(s_maxVertices);

	m_entriesGpuByName = vx::sorted_array<vx::StringID, U32>(s_markersGpu, pAllocator);
	m_entriesGpu = std::make_unique<EntryGpu[]>(s_markersGpu);

	for (auto i = 0u; i < s_markersGpu; ++i)
	{
		glCreateQueries(GL_TIMESTAMP, 1, &m_gpuThreadInfo.markers[i].id_query_start);
		glCreateQueries(GL_TIMESTAMP, 1, &m_gpuThreadInfo.markers[i].id_query_end);
	}

	vx::gl::BufferDescription vboDesc;
	vboDesc.bufferType = vx::gl::BufferType::Array_Buffer;
	vboDesc.flags = vx::gl::BufferStorageFlags::Write | vx::gl::BufferStorageFlags::Dynamic_Storage;
	vboDesc.immutable = 1;
	vboDesc.size = sizeof(Vertex) * s_maxVertices;
	m_vbo.create(vboDesc);

	std::unique_ptr<U32[]> pIndices = std::make_unique<U32[]>(s_maxIndices);
	for (U32 i = 0, j = 0; i < s_maxIndices; i += 6, j += 4)
	{
		pIndices[i] = j;
		pIndices[i + 1] = j + 1;
		pIndices[i + 2] = j + 2;

		pIndices[i + 3] = j + 2;
		pIndices[i + 4] = j + 3;
		pIndices[i + 5] = j;
	}

	vx::gl::BufferDescription iboDesc;
	iboDesc.bufferType = vx::gl::BufferType::Element_Array_Buffer;
	iboDesc.flags = vx::gl::BufferStorageFlags::None;
	iboDesc.immutable = 1;
	iboDesc.size = sizeof(U32) * s_maxIndices;
	iboDesc.pData = pIndices.get();
	m_ibo.create(iboDesc);

	m_vao.create();
	m_vao.bindIndexBuffer(m_ibo);

	// vx::float3 inputPosition;
	m_vao.enableArrayAttrib(0);
	m_vao.arrayAttribFormatF(0, 3, 0, 0);
	m_vao.arrayAttribBinding(0, 0);

	// vx::float3 inputTexCoords;
	m_vao.enableArrayAttrib(1);
	m_vao.arrayAttribFormatF(1, 3, 0, sizeof(vx::float3));
	m_vao.arrayAttribBinding(1, 0);

	// vx::float4 inputColor;
	m_vao.enableArrayAttrib(2);
	m_vao.arrayAttribFormatF(2, 4, 0, sizeof(vx::float3) * 2);
	m_vao.arrayAttribBinding(2, 0);

	m_vao.bindVertexBuffer(m_vbo, 0, 0, sizeof(Vertex));

	return true;
}

void GpuProfiler::updateBuffer(const U32 ascii_code, const vx::float2 &position_x_texSlice, const vx::uint2a &textureSize, const __m128 &invTextureSize, const __m128 &color,
	vx::uint2 *bufferIndex, vx::float2 *cursorPos)
{
	const F32 scale = 0.25f;
	const __m128 vScale = { scale, scale, 0, 0 };

	const __m128 tmp = { -1.0f, 0, 0, 0 };

	if (ascii_code == '\n')
	{
		cursorPos->y -= 53.0f * scale;
		cursorPos->x = position_x_texSlice.x;
	}
	else
	{
		__m128 vCursorPos = { cursorPos->x, cursorPos->y, 0.0f, 0.0f };

		auto pAtlasEntry = m_pFont->getAtlasEntry(ascii_code);
		vx::float4a texRect(pAtlasEntry->x, textureSize.y - pAtlasEntry->y - pAtlasEntry->height, pAtlasEntry->width, pAtlasEntry->height);

		__m128 vAtlasPos = { pAtlasEntry->offsetX, pAtlasEntry->offsetY, 0.0f, 0.0f };

		__m128 vEntrySize = { texRect.z, -texRect.w, 0.0f, 0.0f };

		__m128 vCurrentPosition = _mm_div_ps(vEntrySize, vx::g_VXTwo);
		vCurrentPosition = _mm_add_ps(vCurrentPosition, vAtlasPos);
		vCurrentPosition = _mm_fmadd_ps(vCurrentPosition, vScale, vCursorPos);
		vCurrentPosition = _mm_movelh_ps(vCurrentPosition, tmp);

		__m128 vSource = _mm_mul_ps(texRect, invTextureSize);
		__m128 vSourceSize = _mm_shuffle_ps(vSource, vSource, _MM_SHUFFLE(3, 2, 3, 2));

		static const __m128 texOffsets[4] =
		{
			{ 0, 0, 0, 0 },
			{ 1, 0, 0, 0 },
			{ 1, 1, 0, 0 },
			{ 0, 1, 0, 0 }
		};

		static const __m128 posOffsets[4] =
		{
			{ -0.5f, -0.5f, 0, 0 },
			{ 0.5f, -0.5f, 0, 0 },
			{ 0.5f, 0.5f, 0, 0 },
			{ -0.5f, 0.5f, 0, 0 }
		};

		vEntrySize = _mm_shuffle_ps(texRect, texRect, _MM_SHUFFLE(3, 2, 3, 2));

		for (U32 k = 0; k < 4; ++k)
		{
			U32 index = bufferIndex->x + k;

			auto pos = _mm_mul_ps(posOffsets[k], vEntrySize);
			pos = _mm_fmadd_ps(pos, vScale, vCurrentPosition);
			_mm_storeu_ps(&m_pVertices[index].inputPosition.x, pos);

			__m128 uv = _mm_fmadd_ps(texOffsets[k], vSourceSize, vSource);
			_mm_storeu_ps(&m_pVertices[index].inputTexCoords.x, uv);
			m_pVertices[index].inputTexCoords.z = position_x_texSlice.y;

			_mm_storeu_ps(&m_pVertices[index].inputColor.x, color);
		}
		bufferIndex->x += 4;
		bufferIndex->y += 6;

		cursorPos->x = fmaf(pAtlasEntry->advanceX, scale, cursorPos->x);
	}
}

void GpuProfiler::writeBuffer(I32 strSize, const char* buffer, const vx::float2 &position_x_texSlice, const vx::uint2a &textureSize, const __m128 &vInvTexSize, const __m128 &color,
	vx::uint2* bufferIndex, vx::float2* cursorPos)
{
	for (I32 i = 0; i < strSize; ++i)
	{
		char ascii_code = buffer[i];

		updateBuffer(ascii_code, position_x_texSlice, textureSize, vInvTexSize, color, bufferIndex, cursorPos);
	}
};

void GpuProfiler::writeGpuMarkers(F32 textureSlice, const vx::uint2a &textureSize, const __m128 &vInvTexSize, vx::uint2* bufferIndex, vx::float2 *cursorPos)
{
	const __m128 vColor = { 0.8f, 0.0f, 0.8f, 1 };
	char buffer[s_maxGpuStringSize];
	vx::float2 position_x_texSlice(cursorPos->x, textureSlice);

	for (auto i = 0u; i < m_entryGpuCount; ++i)
	{
		auto &entry = m_entriesGpu[i];

		//int strSize = sprintf_s(buffer, "%s %.6f ms\n",it.name, it.time * 1.0e-6);
		int strSize = sprintf_s(buffer, "%s", entry.name);

		auto layerOffset = entry.layer * 10.0f;
		//cursorPos->x = fmaf(it.layer, 20.0f, cursorPos->x);

		auto currentPos = *cursorPos;

		currentPos.x += layerOffset;
		writeBuffer(strSize, buffer, position_x_texSlice, textureSize, vInvTexSize, vColor, bufferIndex, &currentPos);

		strSize = sprintf_s(buffer, "%.6f ms", entry.time * 1.0e-6);
		currentPos.x = cursorPos->x + 150;
		writeBuffer(strSize, buffer, position_x_texSlice, textureSize, vInvTexSize, vColor, bufferIndex, &currentPos);

		strSize = sprintf_s(buffer, "%.6f ms\n", entry.timeMin * 1.0e-6);
		entry.timeMin = 0xFFFFFFFF;

		currentPos.x = cursorPos->x + 275;
		writeBuffer(strSize, buffer, position_x_texSlice, textureSize, vInvTexSize, vColor, bufferIndex, &currentPos);

		*cursorPos = currentPos;
	}
}

void GpuProfiler::update(F32 dt)
{
	static F32 timer = 0.1f;

	timer -= dt;

	if (timer <= 0.0f)
	{
		timer = 0.1f;

		F32 textureSlice = m_pFont->getTextureEntry().getSlice();
		auto textureSize = m_pFont->getTextureEntry().getTextureSize();
		vx::float2a invTextureSize = 1.0f / static_cast<vx::float2a>(textureSize);//vx::float2(1.0f / textureSize.x, 1.0f / textureSize.y);
		__m128 vInvTexSize = vx::loadFloat(invTextureSize);
		vInvTexSize = _mm_shuffle_ps(vInvTexSize, vInvTexSize, _MM_SHUFFLE(1, 0, 1, 0));

		vx::uint2 bufferIndex = { 0, 0 };
		vx::float2 position = s_position;
		writeGpuMarkers(textureSlice, textureSize, vInvTexSize, &bufferIndex, &position);

		glNamedBufferSubData(m_vbo.getId(), 0, sizeof(Vertex) * bufferIndex.x, m_pVertices.get());

		m_indexCount = bufferIndex.y;
	}
}

void GpuProfiler::render()
{
	auto fsProgram = m_pPipeline->getFragmentShader();
	glProgramUniform1ui(fsProgram, 0, m_textureIndex);
	vx::gl::StateManager::bindPipeline(m_pPipeline->getId());
	vx::gl::StateManager::bindVertexArray(m_vao);
	glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, 0);
}

void GpuProfiler::frame()
{
	++m_currentFrame;

	m_gpuThreadInfo.m_pushedMarkers = 0;

	I64 displayed_frame = m_currentFrame - s_numFramesDelay - 1;
	if (displayed_frame < 0) // don't draw anything during the first frames
		return;

	for (auto i = 0u; i < s_markersGpu; ++i)
	{
		auto &marker = m_gpuThreadInfo.markers[i];

		if (marker.frame == displayed_frame)
		{
			GLint start_ok = 0;
			GLint end_ok = 0;
			glGetQueryObjectiv(marker.id_query_start, GL_QUERY_RESULT_AVAILABLE, &start_ok);
			glGetQueryObjectiv(marker.id_query_end, GL_QUERY_RESULT_AVAILABLE, &end_ok);
			bool ok = (bool)(start_ok && end_ok);

			if (ok)
			{
				U64 start, end;
				glGetQueryObjectui64v(marker.id_query_start, GL_QUERY_RESULT, &start);
				glGetQueryObjectui64v(marker.id_query_end, GL_QUERY_RESULT, &end);

				marker.end = 0;

				vx::StringID sid = vx::make_sid(marker.name);
				auto it = m_entriesGpuByName.find(sid);

				if (it == m_entriesGpuByName.end())
				{
					EntryGpu entry;
					strncpy_s(entry.name, marker.name, s_maxCharacters);
					entry.layer = marker.layer;

					m_entriesGpu[m_entryGpuCount] = entry;

					it = m_entriesGpuByName.insert(sid, m_entryGpuCount);

					++m_entryGpuCount;
				}
				auto &entry = m_entriesGpu[*it];
				entry.layer = marker.layer;
				entry.time = end - start;

				entry.timeMax = fmax(entry.timeMin, entry.timeMax);
				entry.timeMin = fmin(entry.timeMin, entry.time);
			}
		}
	}
}

void GpuProfiler::pushGpuMarker(const char *name)
{
	GpuThreadInfo& ti = m_gpuThreadInfo;
	GpuMarker& marker = ti.markers[ti.currentWriteId];
	VX_ASSERT(ti.currentWriteId < 255u);

	glQueryCounter(marker.id_query_start, GL_TIMESTAMP);

	// Fill in marker
	marker.start = 0;
	marker.end = 0;
	marker.layer = ti.m_pushedMarkers;
	strncpy_s(marker.name, name, s_maxCharacters);
	marker.frame = m_currentFrame;
	incrementCycle(&ti.currentWriteId, s_markersGpu);
	++ti.m_pushedMarkers;
}

void GpuProfiler::popGpuMarker()
{
	GpuThreadInfo& ti = m_gpuThreadInfo;
	// Get the most recent marker that has not been closed yet
	//auto index = ti.currentWriteId - 1;
	U32 index = (ti.currentWriteId == 0) ? s_markersGpu - 1 : ti.currentWriteId - 1;
	//VX_ASSERT(index != -1, "Invalid index !");
	while (ti.markers[index].end != 0) // skip closed markers
		decrementCycle(&index, (U32)s_markersGpu);

	GpuMarker& marker = ti.markers[index];

	marker.end = 1;
	glQueryCounter(marker.id_query_end, GL_TIMESTAMP);

	--ti.m_pushedMarkers;
}