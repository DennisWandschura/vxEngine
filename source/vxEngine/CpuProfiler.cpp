#include "CpuProfiler.h"
#include <vxLib/Container/sorted_vector.h>
#include <vxLib/StringID.h>
#include <vxLib/memory.h>
#include <Windows.h>
#include "Font.h"

class CpuProfiler::Profiler
{
	static const u32 s_maxMarkersPerThread = 128;

	static const u8 s_maxCharacters = 31u;

	static const u8 s_maxCpuStringSize = 64;
	static const u32 s_maxCpuCharacters = s_maxCpuStringSize * s_maxMarkersPerThread;
	static const u32 s_maxVertices = s_maxCpuCharacters * 4u;
	static const u32 s_maxIndices = s_maxCpuCharacters * 6u;

	static LARGE_INTEGER s_frequency;

	struct Marker
	{
		s64 start;
		s64 end;
		char name[s_maxCharacters];
		u8 layer;
	};

	int m_pushedMarkers;
	int m_currentWriteId;
	vx::sorted_vector<vx::StringID, u32> m_entriesGpuByName;
	std::unique_ptr<Marker[]> m_markers;
	std::unique_ptr<Vertex[]> m_pVertices;
	const Font* m_pFont;

	inline void	incrementCycle(s32* pval, size_t array_size)
	{
		s32 val = *pval;
		++val;

		if (val >= (s32)(array_size))
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

	void updateBuffer(const u32 ascii_code, const vx::float2 &position_x_texSlice, const vx::uint2a &textureSize, const __m128 &invTextureSize, const __m128 &color,
		vx::uint2 *bufferIndex, vx::float2 *cursorPos)
	{
		const f32 scale = 0.25f;
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
			vCurrentPosition = vx::fma(vCurrentPosition, vScale, vCursorPos);
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

			for (u32 k = 0; k < 4; ++k)
			{
				u32 index = bufferIndex->x + k;

				auto pos = _mm_mul_ps(posOffsets[k], vEntrySize);
				pos = vx::fma(pos, vScale, vCurrentPosition);
				_mm_storeu_ps(&m_pVertices[index].inputPosition.x, pos);

				__m128 uv = vx::fma(texOffsets[k], vSourceSize, vSource);
				_mm_storeu_ps(&m_pVertices[index].inputTexCoords.x, uv);
				m_pVertices[index].inputTexCoords.z = position_x_texSlice.y;

				_mm_storeu_ps(&m_pVertices[index].inputColor.x, color);
			}
			bufferIndex->x += 4;
			bufferIndex->y += 6;

			cursorPos->x = fmaf(pAtlasEntry->advanceX, scale, cursorPos->x);
		}
	}

	void writeBuffer(s32 strSize, const char* buffer, const vx::float2 &position_x_texSlice, const vx::uint2a &textureSize, const __m128 &vInvTexSize, const __m128 &color,
		vx::uint2* bufferIndex, vx::float2* cursorPos)
	{
		for (s32 i = 0; i < strSize; ++i)
		{
			char ascii_code = buffer[i];

			updateBuffer(ascii_code, position_x_texSlice, textureSize, vInvTexSize, color, bufferIndex, cursorPos);
		}
	}

	void writeGpuMarkers(f32 textureSlice, const vx::uint2a &textureSize, const __m128 &vInvTexSize, vx::uint2* bufferIndex, vx::float2 *cursorPos)
	{
		const __m128 vColor = { 0.8f, 0.0f, 0.8f, 1 };
		char buffer[s_maxCpuStringSize];
		vx::float2 position_x_texSlice(cursorPos->x, textureSlice);

		/*for (auto i = 0u; i < m_entryGpuCount; ++i)
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
		}*/
	}

public:
	void initialize()
	{
		m_entriesGpuByName.reserve(s_maxMarkersPerThread);
		m_markers = vx::make_unique<Marker[]>(s_maxMarkersPerThread);
		m_pVertices = vx::make_unique<Vertex[]>(s_maxVertices);
	}

	void frame()
	{
		m_pushedMarkers = 0;
	}

	void pushMarker(const char* id)
	{
		Marker &marker = m_markers[m_currentWriteId];

		LARGE_INTEGER current;
		QueryPerformanceCounter(&current);

		marker.start = current.QuadPart;
		marker.end = 0;
		marker.layer = m_pushedMarkers;
		strncpy(marker.name, id, s_maxCharacters);
		incrementCycle(&m_currentWriteId, s_maxMarkersPerThread);
		++m_pushedMarkers;
	}

	void popMarker()
	{
		u32 index = (m_currentWriteId == 0) ? s_maxMarkersPerThread - 1 : m_currentWriteId - 1;

		while (m_markers[index].end != 0) // skip closed markers
			decrementCycle(&index, (u32)s_maxMarkersPerThread);

		auto& marker = m_markers[index];

		LARGE_INTEGER current;
		QueryPerformanceCounter(&current);

		marker.end = current.QuadPart;

		--m_pushedMarkers;
	}
};

thread_local CpuProfiler::Profiler* CpuProfiler::s_profiler{ nullptr };

void CpuProfiler::initialize()
{
	if (s_profiler == nullptr)
	{
		s_profiler = new Profiler();
		s_profiler->initialize();
	}
}

void CpuProfiler::shutdown()
{
	if (s_profiler != nullptr)
	{
		delete s_profiler;
	}
}

void CpuProfiler::frame()
{
	s_profiler->frame();
}

void CpuProfiler::pushMarker(const char* id)
{
	s_profiler->pushMarker(id);
}

void CpuProfiler::popMarker()
{
	s_profiler->popMarker();
}