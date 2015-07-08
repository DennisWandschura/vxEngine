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
#include "BlendFuncCommand.h"
#include <vxGL/gl.h>
#include "../Segment.h"
#include <vxEngineLib/ParserNode.h>
#include "../CommandFactory.h"

namespace Graphics
{
	void createFromNodeBlendFuncCommand(const Parser::Node &node, Segment* segment, void*)
	{
		auto paramsNode = node.get("params");

		u32 param[2];
		paramsNode->as(0, &param[0]);
		paramsNode->as(1, &param[1]);

		BlendFuncCommand command;
		command.set(param[0], param[1]);

		segment->pushCommand(command);
	}

	REGISTER_COMMANDFACTORY(BlendFuncCommand, createFromNodeBlendFuncCommand);

	void BlendFuncCommand::set(u32 sfactor, u32 dfactor)
	{
		m_sfactor = sfactor;
		m_dfactor = dfactor;
	}

	void BlendFuncCommand::execute(const u8* p, u32* offset)
	{
		BlendFuncCommand* cmd = (BlendFuncCommand*)p;

		glBlendFunc(cmd->m_sfactor, cmd->m_dfactor);

		*offset += sizeof(BlendFuncCommand);
	}
}