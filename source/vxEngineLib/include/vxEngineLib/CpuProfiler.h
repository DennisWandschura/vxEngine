#pragma once

#include <vxlib/types.h>
#include <memory>
#include <vxLib/Container/sorted_vector.h>
#include <vxLib/StringID.h>

class CpuProfiler
{
	static const u8 s_maxCharacters = 31u;
	static const u32 s_maxMarkersPerThread = 128;

	struct Marker;
	struct Entry;

	static u64 s_frequency;

	std::unique_ptr<Marker[]> m_markers;
	std::unique_ptr<Entry[]> m_entries;
	vx::sorted_vector<vx::StringID, u32> m_entriesByName;
	int m_pushedMarkers;
	int m_currentWriteId;
	u32 m_entryCount;

public:
	CpuProfiler();
	~CpuProfiler();

	void initialize();
	void shutdown();

	void update();

	void frame();
	void pushMarker(const char* id);
	void popMarker();
};