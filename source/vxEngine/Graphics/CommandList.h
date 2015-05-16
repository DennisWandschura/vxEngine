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
#pragma once

#include <vxLib/types.h>
#include <vxLib/Container/sorted_vector.h>
#include <vxLib/StringID.h>
#include <vxLib/memory.h>

namespace Graphics
{
	class Segment;

	class CommandList
	{
		struct ColdData
		{
			vx::sorted_vector<u32, Segment> m_inactiveSegments;
			vx::sorted_vector<vx::StringID, u32> m_inactiveSegmentIndices;
		};

		vx::sorted_vector<u32, Segment> m_sortedSegments;
		vx::sorted_vector<vx::StringID, u32> m_segmentIndices;
		std::unique_ptr<ColdData> m_coldData;
	
		void swapSegmentsImpl(u32 a, u32 b);

	public:
		CommandList();
		~CommandList();

		void initialize();

		void pushSegment(const Segment &segment, const char* id);

		void enableSegment(const char* id);
		void disableSegment(const char* id);

		void draw();
	};
}