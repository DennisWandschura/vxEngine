#pragma once

#include <vxLib/types.h>
#include <vxLib/Container/sorted_vector.h>
#include <vxLib/StringID.h>
#include <memory>

namespace Graphics
{
	class Segment;

	class CommandList
	{
		struct ColdData
		{
			vx::sorted_vector<U32, Segment> m_inactiveSegments;
			vx::sorted_vector<vx::StringID, U32> m_inactiveSegmentIndices;
		};

		vx::sorted_vector<U32, Segment> m_sortedSegments;
		vx::sorted_vector<vx::StringID, U32> m_segmentIndices;
		std::unique_ptr<ColdData> m_coldData;
	
		void swapSegmentsImpl(U32 a, U32 b);

	public:
		CommandList();
		~CommandList();

		void initialize();

		void pushSegment(const Segment &segment, const char* id, U32 slot);

		void enableSegment(const char* id);
		void disableSegment(const char* id);

		void draw();
	};
}