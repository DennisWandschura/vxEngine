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
#include "vxRenderAspect/Graphics/Commands/ClearColorCommand.h"
#include <vxGL/StateManager.h>
#include <vxRenderAspect/Graphics/Segment.h>
#include <vxRenderAspect/Graphics/CommandFactory.h>
#include <vxEngineLib/ParserNode.h>

namespace Graphics
{
	void createFromNodeClearColorCommand(const Parser::Node &node, Segment* segment, void*)
	{
		ClearColorCommand cmd;

		auto paramsNode = node.get("params");

		f32 params[4];
		paramsNode->as(0, &params[0]);
		paramsNode->as(1, &params[1]);
		paramsNode->as(2, &params[2]);
		paramsNode->as(3, &params[3]);

		cmd.m_clearColor.x = params[0];
		cmd.m_clearColor.y = params[1];
		cmd.m_clearColor.z = params[2];
		cmd.m_clearColor.w = params[3];

		segment->pushCommand(cmd);
	}

	REGISTER_COMMANDFACTORY(ClearColorCommand, createFromNodeClearColorCommand);

	void ClearColorCommand::set(const vx::float4 &clearColor)
	{
		m_clearColor = clearColor;
	}

	void ClearColorCommand::execute(const u8* p, u32* offset)
	{
		auto data = (ClearColorCommand*)p;
		vx::gl::StateManager::setClearColor(data->m_clearColor.x, data->m_clearColor.y, data->m_clearColor.z, data->m_clearColor.w);

		*offset += sizeof(ClearColorCommand);
	}
}