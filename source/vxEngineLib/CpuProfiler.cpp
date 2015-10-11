#include <vxEngineLib/CpuProfiler.h>
#include <Windows.h>
#include <vxEngineLib/Graphics/RenderAspectInterface.h>
#include <vxEngineLib/CpuTimer.h>

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
	m_lastEntryCount(0),
	m_entryCount(0)
{

}

CpuProfiler::~CpuProfiler()
{

}

void CpuProfiler::initialize(const vx::uint2 &position)
{
	if (s_frequency == 0)
	{
		QueryPerformanceFrequency((LARGE_INTEGER*)&s_frequency);
	}

	m_markers = std::make_unique<Marker[]>(s_maxMarkersPerThread);
	m_entries = std::make_unique<Entry[]>(s_maxMarkersPerThread);

	m_entriesByName.reserve(s_maxMarkersPerThread);
	m_position = position;
}

void CpuProfiler::shutdown()
{
	m_entriesByName.clear();
	m_markers.reset();
	m_entries.reset();
}

void CpuProfiler::update(RenderAspectInterface* renderAspect)
{
	RenderUpdateTextData data;

	const f32 yOffset = 15.0f;
	f32 xPos = m_position.x;
	f32 ypos = m_position.y;

	auto strSize = snprintf(data.text, sizeof(data.text),"CPU:");
	data.text[strSize] = '\0';

	data.position.x = xPos;
	data.position.y = ypos;
	data.color = vx::float3(1, 0, 1);
	data.strSize = strSize;
	renderAspect->queueUpdate(RenderUpdateTaskType::UpdateText, (u8*)&data, sizeof(data));

	ypos -= yOffset;

	for (u32 i = 0; i < m_entryCount; ++i)
	{
		auto &entry = m_entries[i];

		auto time = f32(entry.time * 0.001f);
		auto maxTime = f32(entry.maxTime * 0.001f);

		entry.maxTime = 0;

		auto strSize = snprintf(data.text, sizeof(data.text), "%s %.3f %.3f ms", entry.name, time, maxTime);
		strSize = (strSize < 0) ? sizeof(data.text) : strSize;
		data.text[strSize] = '\0';

		data.position.x = xPos + entry.layer * yOffset;
		data.position.y = ypos;
		data.color = vx::float3(1, 0, 1);
		data.strSize = strSize;

		renderAspect->queueUpdate(RenderUpdateTaskType::UpdateText, (u8*)&data, sizeof(data));

		ypos -= yOffset;
	}

	if (m_lastEntryCount != m_entryCount)
	{
		m_lastEntryCount = m_entryCount;
		m_entriesByName.clear();
		m_entryCount = 0;
	}
}

void CpuProfiler::updateCpuUsage()
{
	/*u64 creationTime, exitTime, kernelTime, userTime;
	auto hr = GetThreadTimes(m_threadHandle, (FILETIME*)&creationTime, (FILETIME*)&exitTime, (FILETIME*)&kernelTime, (FILETIME*)&userTime);

	u64 threadTime = kernelTime + userTime;

	FILETIME systemIdle;
	u64 systemKernel, systemUser;
	GetSystemTimes(&systemIdle, (FILETIME*)&systemKernel, (FILETIME*)&systemUser);
	u64 systemTime = systemKernel + systemUser;

	u64 diffThread = threadTime - m_lastTime;
	u64 diffSystem = systemTime - m_lastSystem;

	auto cpuUsage = (100.0 * diffThread) / diffSystem;
	//printf("%f\n", cpuUsage);

	m_lastTime = threadTime;
	m_lastSystem = systemTime;*/
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
			strncpy(entry.name, marker.name, s_maxCharacters);
			entry.layer = marker.layer;
			entry.time = (marker.end - marker.start) * 1000000 / s_frequency;
			entry.maxTime = std::max(entry.maxTime, entry.time);

			marker.end = 0;
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