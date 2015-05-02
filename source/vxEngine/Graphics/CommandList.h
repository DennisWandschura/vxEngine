#pragma once

#include <vxLib/types.h>
#include <vector>
#include <vxLib/Container/sorted_vector.h>
#include <vxLib/StringID.h>

namespace Graphics
{
	class Segment;

	class CommandList
	{
		std::vector<Segment> m_segments;
		vx::sorted_vector<vx::StringID64, U32> m_segmentIndices;

		void swapSegmentsImpl(U32 a, U32 b);

	public:
		CommandList();
		~CommandList();

		void pushSegment(const Segment &segment, const char* id);

		void eraseSegment(const char* id);
		void swapSegments(const char* segmentA, const char* segmentB);
		void swapSegments(U32 a, U32 b);

		void draw();
	};
}