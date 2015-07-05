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
#include "vxRenderAspect/GpuProfiler.h"
#include "vxRenderAspect/Font.h"
#include <Windows.h>
#include <vxLib/memory.h>
#include <vxGL/gl.h>
#include <vxRenderAspect/Graphics/TextRenderer.h>

namespace
{
	inline void	incrementCycle(u8* pval, u8 array_size)
	{
		s32 val = *pval;
		++val;

		if (val >= (s32)(array_size))
			val = 0;

		*pval = val;
	}

	inline void	incrementCycle(s32* pval, size_t array_size)
	{
		s32 val = *pval;
		++val;

		if (val >= (s32)(array_size))
			val = 0;

		*pval = val;
	}

	inline void	incrementCycle(s64* pval, size_t array_size)
	{
		s64 val = *pval;
		++val;

		if (val >= (s64)(array_size))
			val = 0;

		*pval = val;
	}

	inline void	incrementCycle(u32* pval, size_t array_size)
	{
		s64 val = *pval;
		++val;

		if (val >= (s64)(array_size))
			val = 0;

		*pval = val;
	}

	inline void	decrementCycle(u32* pval, size_t array_size)
	{
		s64 val = *pval;
		--val;

		if (val < 0)
			val = (s64)(array_size - 1);

		*pval = val;
	}

	inline void	decrementCycle(s64* pval, size_t array_size)
	{
		s64 val = *pval;
		--val;

		if (val < 0)
			val = (s64)(array_size - 1);

		*pval = val;
	}
}

struct GpuProfiler::EntryGpu
{
	u32 time{ 0 };
	u32 timeMin{ 0xffffffff };
	u32 timeMax{ 0 };
	char name[s_maxCharacters];
	u8 layer{ 0 };
	u32 queryStart{ 0 };
	u32 queryEnd{ 0 };

	EntryGpu()
	{
		name[0] = '\0';
	}
};

GpuProfiler::GpuProfiler()
	:m_currentFrame(0),
	m_gpuThreadInfo(),
	m_entriesGpuByName(),
	m_entryGpuCount(0),
	m_entriesGpu(),
	m_textRenderer(nullptr),
	m_position(0, 0)
{
}

GpuProfiler::~GpuProfiler()
{
}

bool GpuProfiler::initialize(const vx::float2 &position, Graphics::TextRenderer* textRenderer, vx::StackAllocator* pAllocator)
{
	m_entriesGpuByName = vx::sorted_array<vx::StringID, u32>(s_markersGpu, pAllocator);
	m_entriesGpu = vx::make_unique<EntryGpu[]>(s_markersGpu);

	for (auto i = 0u; i < s_markersGpu; ++i)
	{
		glCreateQueries(GL_TIMESTAMP, 1, &m_gpuThreadInfo.markers[i].id_query_start);
		glCreateQueries(GL_TIMESTAMP, 1, &m_gpuThreadInfo.markers[i].id_query_end);
	}

	m_textRenderer = textRenderer;
	m_position = position;

	return true;
}

void GpuProfiler::update()
{
	char buffer[64];

	vx::float2 position = m_position;
	vx::float2 currentPosition = m_position;
	for (auto i = 0u; i < m_entryGpuCount; ++i)
	{
		auto &entry = m_entriesGpu[i];
		auto entryName = entry.name;
		auto layer = entry.layer;
		f32 time = entry.time * 1.0e-6;

		int strSize = sprintf_s(buffer, "%s %.6f", entryName, time);
		std::string text;
		text.reserve(strSize);
		text.assign(buffer, buffer + strSize);

		auto layerOffset = layer * 10.0f;

		currentPosition.x = position.x + layerOffset;
		m_textRenderer->pushEntry(std::move(text), currentPosition, vx::float3(1, 0, 1));

		currentPosition.y -= 15;

		/*auto layerOffset = entry.layer * 10.0f;

		auto currentPos = *cursorPos;
		currentPos.x += layerOffset;

		strSize = sprintf_s(buffer, "%.6f ms", entry.time * 1.0e-6);
		currentPos.x = cursorPos->x + 150;

		strSize = sprintf_s(buffer, "%.6f ms\n", entry.timeMin * 1.0e-6);

		currentPos.x = cursorPos->x + 275;

		*cursorPos = currentPos;*/
		entry.timeMin = 0xFFFFFFFF;
	}
}

void GpuProfiler::frame()
{
	++m_currentFrame;

	m_gpuThreadInfo.m_pushedMarkers = 0;

	s64 displayed_frame = m_currentFrame - s_numFramesDelay - 1;
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
				u64 start, end;
				glGetQueryObjectui64v(marker.id_query_start, GL_QUERY_RESULT, &start);
				glGetQueryObjectui64v(marker.id_query_end, GL_QUERY_RESULT, &end);

				marker.end = 0;

				vx::StringID sid = vx::make_sid(marker.name);
				auto it = m_entriesGpuByName.find(sid);

				if (it == m_entriesGpuByName.end())
				{
					EntryGpu entry;
					strncpy(entry.name, marker.name, s_maxCharacters);
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
	strncpy(marker.name, name, s_maxCharacters);
	marker.frame = m_currentFrame;
	incrementCycle(&ti.currentWriteId, s_markersGpu);
	++ti.m_pushedMarkers;
}

void GpuProfiler::popGpuMarker()
{
	GpuThreadInfo& ti = m_gpuThreadInfo;
	// Get the most recent marker that has not been closed yet
	//auto index = ti.currentWriteId - 1;
	u32 index = (ti.currentWriteId == 0) ? s_markersGpu - 1 : ti.currentWriteId - 1;
	//VX_ASSERT(index != -1, "Invalid index !");
	while (ti.markers[index].end != 0) // skip closed markers
		decrementCycle(&index, (u32)s_markersGpu);

	GpuMarker& marker = ti.markers[index];

	marker.end = 1;
	glQueryCounter(marker.id_query_end, GL_TIMESTAMP);

	--ti.m_pushedMarkers;
}