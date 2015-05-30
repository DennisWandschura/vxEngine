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
#include "ClearCommand.h"
#include <vxLib/gl/gl.h>
#include "../Segment.h"
#include "../../ParserNode.h"
#include "../CommandFactory.h"

namespace Graphics
{
	void createFromNodeClearCommand(const Parser::Node &node, Segment* segment, void* p)
	{
		auto paramsNode = node.get("params");

		u32 param;
		paramsNode->as(0, &param);

		ClearCommand command;
		command.set(param);

		segment->pushCommand(command);
	}

	REGISTER_COMMANDFACTORY(ClearCommand, createFromNodeClearCommand);

	void ClearCommand::set(u32 bits)
	{
		m_bits = bits;
	}

	void ClearCommand::execute(u32* offset)
	{
		glClear(m_bits);

		*offset += sizeof(ClearCommand);
	}
}