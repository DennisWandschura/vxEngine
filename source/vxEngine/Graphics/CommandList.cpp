#include "CommandList.h"
#include "Segment.h"

namespace Graphics
{
	CommandList::CommandList()
		:m_segments()
	{

	}

	CommandList::~CommandList()
	{

	}

	void CommandList::pushSegment(const Segment &segment, const char* id)
	{
		auto index = m_segments.size();
		m_segments.push_back(segment);

		m_segmentIndices.insert(vx::make_sid(id), index);
	}

	void CommandList::eraseSegment(const char* id)
	{
		auto sid = vx::make_sid(id);
		auto it = m_segmentIndices.find(sid);
		if (it != m_segmentIndices.end())
		{
			auto segmentIndex = *it;

			m_segments.erase(m_segments.begin() + segmentIndex);

			m_segmentIndices.erase(it);
		}
	}

	void CommandList::swapSegments(const char* segmentA, const char* segmentB)
	{
		auto sida = vx::make_sid(segmentA);
		auto sidb = vx::make_sid(segmentB);

		auto ita = m_segmentIndices.find(sida);
		auto itb = m_segmentIndices.find(sidb);

		if (ita == m_segmentIndices.end() || itb == m_segmentIndices.end())
			return;

		U32 a = *ita;
		U32 b = *itb;

		swapSegmentsImpl(a, b);
	}

	void CommandList::swapSegments(U32 a, U32 b)
	{
		auto size = m_segments.size();
		if (a >= size || b >= size)
			return;

		swapSegmentsImpl(a, b);
	}

	void CommandList::swapSegmentsImpl(U32 a, U32 b)
	{
		std::swap(m_segmentIndices[a], m_segmentIndices[b]);
		std::swap(m_segments[a], m_segments[b]);
	}

	void CommandList::draw()
	{
		for (auto &it : m_segments)
		{
			it.draw();
		}
	}
}