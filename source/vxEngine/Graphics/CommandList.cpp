#include "CommandList.h"
#include "Segment.h"

namespace Graphics
{
	CommandList::CommandList()
		:m_sortedSegments(),
		m_segmentIndices(),
		m_coldData()
	{
	}

	CommandList::~CommandList()
	{

	}

	void CommandList::initialize()
	{
		m_coldData = std::make_unique<ColdData>();
	}

	void CommandList::pushSegment(const Segment &segment, const char* id, U32 slot)
	{
		m_sortedSegments.insert(slot, segment);
		m_segmentIndices.insert(vx::make_sid(id), slot);
	}

	void CommandList::enableSegment(const char* id)
	{
		auto sid = vx::make_sid(id);
		auto it = m_coldData->m_inactiveSegmentIndices.find(sid);
		if (it != m_coldData->m_inactiveSegmentIndices.end())
		{
			auto segmentIndex = *it;
			auto itSegment = m_coldData->m_inactiveSegments.find(segmentIndex);

			m_sortedSegments.insert(segmentIndex, std::move(*itSegment));
			m_segmentIndices.insert(sid, segmentIndex);

			m_coldData->m_inactiveSegments.erase(itSegment);
			m_coldData->m_inactiveSegmentIndices.erase(it);
		}
	}

	void CommandList::disableSegment(const char* id)
	{
		auto sid = vx::make_sid(id);
		auto it = m_segmentIndices.find(sid);
		if (it != m_segmentIndices.end())
		{
			auto segmentIndex = *it;
			auto itSegment = m_sortedSegments.find(segmentIndex);

			m_coldData->m_inactiveSegments.insert(segmentIndex, std::move(*itSegment));
			m_coldData->m_inactiveSegmentIndices.insert(sid, segmentIndex);

			m_sortedSegments.erase(itSegment);
			m_segmentIndices.erase(it);
		}
	}

	void CommandList::draw()
	{
		for (auto &it : m_sortedSegments)
		{
			it.draw();
		}
	}
}