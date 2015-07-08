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

#include "CpuProfileCommand.h"
//#include "../../CpuProfiler.h"
#include "../CommandFactory.h"
#include <vxEngineLib/ParserNode.h>
#include "../Segment.h"

namespace Graphics
{
	/*void createFromNodeCpuProfilePushCommand(const Parser::Node &node, Segment* segment, void*)
	{
		auto paramsNode = node.get("params");

		std::string param;
		paramsNode->as(0, &param);

		CpuProfilePushCommand command;
		command.set(param.c_str());

		segment->pushCommand(command);
	}

	void createFromNodeCpuProfilePopCommand(const Parser::Node &node, Segment* segment, void*)
	{
		auto paramsNode = node.get("params");

		std::string param;
		paramsNode->as(0, &param);

		CpuProfilePopCommand command;

		segment->pushCommand(command);
	}

	REGISTER_COMMANDFACTORY(CpuProfilePushCommand, createFromNodeCpuProfilePushCommand);
	REGISTER_COMMANDFACTORY(CpuProfilePopCommand, createFromNodeCpuProfilePopCommand);

	void CpuProfilePushCommand::set(const char* name)
	{
		auto size = strlen(name);
		size = std::min(size, 31llu);

		strncpy(m_name, name, size);
		m_name[size] = '\0';
	}

	void CpuProfilePushCommand::execute(const u8* p, u32* offset)
	{
		auto ptr = (CpuProfilePushCommand*)p;

		//CpuProfiler::pushMarker(ptr->m_name);
		*offset = sizeof(CpuProfilePushCommand);
	}

	void CpuProfilePopCommand::execute(const u8*, u32* offset)
	{
		//CpuProfiler::popMarker();
		*offset = sizeof(CpuProfilePopCommand);
	}*/
}