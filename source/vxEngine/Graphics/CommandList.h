#pragma once

#include <memory>
#include <vxLib/types.h>

namespace Graphics
{
	class Segment;

	class CommandList
	{
		std::unique_ptr<Segment[]> m_segments;
		U32 m_count;

	public:
		CommandList();
		~CommandList();

		void createSegments(U32 count);
		void setSegment(U32 i, const Segment &segment);

		void draw();
	};
}