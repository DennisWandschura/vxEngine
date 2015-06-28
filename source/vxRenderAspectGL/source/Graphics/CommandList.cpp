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
#include "vxRenderAspect/Graphics/CommandList.h"
#include "vxRenderAspect/Graphics/Segment.h"

namespace Graphics
{
	void swapSegment(const char* id, 
		vx::sorted_vector<u32, Segment>* segmentSrc, vx::sorted_vector<vx::StringID, u32>* segmentIndicesSrc,
		vx::sorted_vector<u32, Segment>* segmentDst, vx::sorted_vector<vx::StringID, u32>* segmentIndicesDst)
	{
		auto sid = vx::make_sid(id);
		auto it = segmentIndicesSrc->find(sid);
		if (it != segmentIndicesSrc->end())
		{
			auto segmentIndex = *it;
			auto itSegment = segmentSrc->find(segmentIndex);

			segmentDst->insert(segmentIndex, std::move(*itSegment));
			segmentIndicesDst->insert(sid, segmentIndex);

			segmentSrc->erase(itSegment);
			segmentIndicesSrc->erase(it);
		}
	}

	CommandList::CommandList()
		:m_sortedSegments(),
		m_coldData()
	{
	}

	CommandList::CommandList(CommandList &&rhs)
		: m_sortedSegments(std::move(rhs.m_sortedSegments)),
		m_coldData(std::move(rhs.m_coldData))
	{

	}

	CommandList::~CommandList()
	{

	}

	CommandList& CommandList::operator=(CommandList &&rhs)
	{
		if (this != &rhs)
		{
			m_sortedSegments = std::move(rhs.m_sortedSegments);
			m_coldData = std::move(rhs.m_coldData);
		}
		return *this;
	}

	void CommandList::initialize()
	{
		m_coldData = vx::make_unique<ColdData>();
	}

	void CommandList::pushSegment(const Segment &segment, const char* id)
	{
		VX_ASSERT(segment.isValid());
		u32 slot = m_sortedSegments.size();
		auto sid = vx::make_sid(id);

		m_sortedSegments.insert(slot, segment);
		m_coldData->m_segmentIndices.insert(sid, slot);
	}

	void CommandList::enableSegment(const char* id)
	{
		/*auto sid = vx::make_sid(id);
		auto it = m_coldData->m_inactiveSegmentIndices.find(sid);
		if (it != m_coldData->m_inactiveSegmentIndices.end())
		{
			auto segmentIndex = *it;
			auto itSegment = m_coldData->m_inactiveSegments.find(segmentIndex);

			m_sortedSegments.insert(segmentIndex, std::move(*itSegment));
			m_coldData->m_segmentIndices.insert(sid, segmentIndex);

			m_coldData->m_inactiveSegments.erase(itSegment);
			m_coldData->m_inactiveSegmentIndices.erase(it);
		}*/
		swapSegment(id, &m_coldData->m_inactiveSegments, &m_coldData->m_inactiveSegmentIndices, &m_sortedSegments, &m_coldData->m_segmentIndices);
	}

	void CommandList::disableSegment(const char* id)
	{
		/*auto sid = vx::make_sid(id);
		auto it = m_coldData->m_segmentIndices.find(sid);
		if (it != m_coldData->m_segmentIndices.end())
		{
			auto segmentIndex = *it;
			auto itSegment = m_sortedSegments.find(segmentIndex);

			m_coldData->m_inactiveSegments.insert(segmentIndex, std::move(*itSegment));
			m_coldData->m_inactiveSegmentIndices.insert(sid, segmentIndex);

			m_sortedSegments.erase(itSegment);
			m_coldData->m_segmentIndices.erase(it);
		}*/
		swapSegment(id, &m_sortedSegments, &m_coldData->m_segmentIndices, &m_coldData->m_inactiveSegments, &m_coldData->m_inactiveSegmentIndices);
	}

	void CommandList::clear()
	{
		m_sortedSegments.clear();
		m_coldData->m_segmentIndices.clear();
		m_coldData->m_inactiveSegments.clear();
		m_coldData->m_inactiveSegmentIndices.clear();
	}

	void CommandList::draw() const
	{
		for (auto &it : m_sortedSegments)
		{
			it.draw();
		}
	}

	u32 CommandList::size() const
	{
		return m_sortedSegments.size();
	}
}