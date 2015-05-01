#include "CommandList.h"
#include "Segment.h"

namespace Graphics
{
	CommandList::CommandList()
		:m_segments(),
		m_count(0)
	{

	}

	CommandList::~CommandList()
	{

	}

	void CommandList::createSegments(U32 count)
	{
		m_segments = std::make_unique<Segment[]>(count);
		m_count = count;
	}

	void CommandList::setSegment(U32 i, const Segment &segment)
	{
		m_segments[i] = segment;
	}

	void CommandList::draw()
	{
		for (auto i = 0u; i < m_count; ++i)
		{
			m_segments[i].draw();
		}
	}
}