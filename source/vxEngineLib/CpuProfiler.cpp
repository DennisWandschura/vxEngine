#include <vxEngineLib/CpuProfiler.h>
#include <Windows.h>

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

struct CpuProfiler::Marker
{
	s64 start;
	s64 end;
	char name[s_maxCharacters];
	u8 layer;
};

struct CpuProfiler::Entry
{
	char name[s_maxCharacters];
	u8 layer;
	s64 time;
	s64 maxTime;
};

u64 CpuProfiler::s_frequency{0};

CpuProfiler::CpuProfiler()
	:m_markers(),
	m_entries(),
	m_entriesByName(),
	m_pushedMarkers(0),
	m_currentWriteId(0),
	m_entryCount(0)
{

}

CpuProfiler::~CpuProfiler()
{

}

void CpuProfiler::initialize()
{
	if (s_frequency == 0)
	{
		QueryPerformanceFrequency((LARGE_INTEGER*)&s_frequency);
	}

	m_markers = std::make_unique<Marker[]>(s_maxMarkersPerThread);
	m_entries = std::make_unique<Entry[]>(s_maxMarkersPerThread);

	m_entriesByName.reserve(s_maxMarkersPerThread);
}

void CpuProfiler::shutdown()
{
	m_entriesByName.clear();
	m_markers.reset();
	m_entries.reset();
}

void CpuProfiler::update()
{
	/*f32 textureSlice = m_pFont->getTextureEntry().getSlice();
	auto textureSize = m_pFont->getTextureEntry().getTextureSize();
	vx::float4a invTextureSize;
	invTextureSize.x = 1.0f / textureSize.x;
	invTextureSize.y = 1.0f / textureSize.y;

	//vx::float2(1.0f / textureSize.x, 1.0f / textureSize.y);
	auto vInvTexSize = _mm_shuffle_ps(invTextureSize.v, invTextureSize.v, _MM_SHUFFLE(1, 0, 1, 0));

	vx::uint2 bufferIndex = { 0, 0 };
	vx::float2 position = m_position;
	vx::lock_guard<vx::mutex> lock(m_mutex);
	writeGpuMarkers(textureSlice, textureSize, vInvTexSize, &bufferIndex, &position);

	//glNamedBufferSubData(m_vbo.getId(), 0, sizeof(Vertex) * bufferIndex.x, m_pVertices.get());

	m_vertexCount = bufferIndex.x;
	m_indexCount = bufferIndex.y;*/
}

void CpuProfiler::frame()
{
	m_pushedMarkers = 0;

	for (u32 i = 0; i < s_maxMarkersPerThread; ++i)
	{
		auto &marker = m_markers[i];

		if (marker.end != 0)
		{
			auto sid = vx::make_sid(marker.name);
			auto it = m_entriesByName.find(sid);

			if (it == m_entriesByName.end())
			{
				Entry entry;
				strncpy(entry.name, marker.name, s_maxCharacters);
				entry.layer = marker.layer;
				entry.maxTime = 0;

				m_entries[m_entryCount] = entry;

				it = m_entriesByName.insert(sid, m_entryCount);

				++m_entryCount;
			}

			auto &entry = m_entries[*it];
			entry.layer = marker.layer;
			entry.time = (marker.end - marker.start) * 1000000 / s_frequency;
			entry.maxTime = std::max(entry.maxTime, entry.time);
		}
	}
}

void CpuProfiler::pushMarker(const char* id)
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

void CpuProfiler::popMarker()
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