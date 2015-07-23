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
#include "ConservativeRasterCommand.h"
#include <vxGL/gl.h>
#include "../Segment.h"
#include <vxEngineLib/ParserNode.h>
#include "../CommandFactory.h"

namespace ConservativeRasterCommandCpp
{
	typedef void(__stdcall *FUNCTIONTYPE)(GLenum);

	FUNCTIONTYPE g_functions[] =
	{
		glDisable,
		glEnable
	};
}

namespace Graphics
{
	void createFromNodeConservativeRasterCommand(const Parser::Node &node, Segment* segment, void*)
	{
		auto paramsNode = node.get("params");

		u32 param;
		paramsNode->as(0, &param);

		ConservativeRasterCommand command;
		command.set(param);

		segment->pushCommand(command);
	}

	CommandFactoryRegister g_commandFactoryConservativeRasterCommand{ "ConservativeRasterCommand", createFromNodeConservativeRasterCommand };

	void ConservativeRasterCommand::set(u32 active)
	{
		m_active = active;
	}

	void ConservativeRasterCommand::execute(const u8* ptr, u32* offset)
	{
		auto data = (ConservativeRasterCommand*)ptr;

		ConservativeRasterCommandCpp::g_functions[data->m_active](GL_CONSERVATIVE_RASTERIZATION_NV);

		*offset += sizeof(ConservativeRasterCommand);
	}
}