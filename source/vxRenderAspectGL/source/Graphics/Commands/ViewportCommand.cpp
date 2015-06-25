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
#include <vxRenderAspect/Graphics/Commands/ViewportCommand.h>
#include <vxGL/StateManager.h>
#include <vxRenderAspect/Graphics/Segment.h>
#include <vxEngineLib/ParserNode.h>
#include <vxRenderAspect/Graphics/CommandFactory.h>

namespace Graphics
{
	void createFromNodeViewportCommand(const Parser::Node &node, Segment* segment, void*)
	{
		auto paramsNode = node.get("params");

		u32 params[4];
		paramsNode->as(0, &params[0]);
		paramsNode->as(1, &params[1]);
		paramsNode->as(2, &params[2]);
		paramsNode->as(3, &params[3]);

		ViewportCommand command;
		command.m_offset.x = params[0];
		command.m_offset.y = params[1];
		command.m_size.x = params[2];
		command.m_size.y = params[3];

		segment->pushCommand(command);
	}

	REGISTER_COMMANDFACTORY(ViewportCommand, createFromNodeViewportCommand);

	void ViewportCommand::set(const vx::uint2 &offset, const vx::uint2 &size)
	{
		m_offset = offset;
		m_size = size;
	}

	void ViewportCommand::execute(const u8* p, u32* offset)
	{
		auto ptr = (ViewportCommand*)p;
		vx::gl::StateManager::setViewport(ptr->m_offset.x, ptr->m_offset.y, ptr->m_size.x, ptr->m_size.y);

		*offset += sizeof(ViewportCommand);

	}
}