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

#include "DrawArraysCommand.h"
#include <vxGL/gl.h>
#include "../Segment.h"
#include <vxEngineLib/ParserNode.h>
#include "../CommandFactory.h"

namespace Graphics
{
	void __fastcall createFromNodeDrawArraysCommand(const Parser::Node &node, Segment* segment, void*)
	{
		auto paramsNode = node.get("params");

		u32 params[3];
		paramsNode->as(0, &params[0]);
		paramsNode->as(1, &params[1]);
		paramsNode->as(2, &params[2]);

		DrawArraysCommand command;
		command.set(params[0], params[1], params[2]);

		segment->pushCommand(command);
	}

	CommandFactoryRegister g_commandFactoryDrawArraysCommand{ "DrawArraysCommand", createFromNodeDrawArraysCommand };

	void DrawArraysCommand::set(u32 mode, u32 first, u32 count)
	{
		m_mode = mode;
		m_first = first;
		m_count = count;
	}

	void DrawArraysCommand::execute(const u8* p, u32* offset)
	{
		auto ptr = (DrawArraysCommand*)p;

		glDrawArrays(ptr->m_mode, ptr->m_first, ptr->m_count);

		*offset += sizeof(DrawArraysCommand);
	}
}