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

#include "vxRenderAspect/Graphics/CommandListFactory.h"
#include <vxEngineLib/ParserNode.h>
#include "vxRenderAspect/SegmentFactory.h"
#include "vxRenderAspect/Graphics/Segment.h"

namespace Graphics
{
	void CommandListFactory::createFromFile(const char *file, const gl::ObjectManager &objectManager, const vx::gl::ShaderManager &shaderManager, CommandList* list)
	{
		Parser::Node root;
		auto result = root.createFromFile(file);
		VX_ASSERT(result == true);

		auto segmentsNode = root.get("segments");

		list->initialize();

		std::string segmentFile;
		auto segmentCount = segmentsNode->size();
		for (u32 i = 0; i < segmentCount; ++i)
		{
			segmentFile.clear();
			segmentsNode->as(i, &segmentFile);
			auto segment = Graphics::SegmentFactory::createFromFile(segmentFile.c_str(), objectManager, shaderManager);

			list->pushSegment(segment, segmentFile.c_str());
		}

		//return std::move(list);
	}
}