#pragma once

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

namespace Parser
{
	class Node;
}

namespace vx
{
	namespace gl
	{
		class ProgramPipeline;
	}
}

#include <vxLib/Container/sorted_vector.h>
#include <vxLib/StringID.h>

namespace Graphics
{
	class Segment;

	struct CommandDataDescription
	{
		u32 count;
		void* p;
	};

	typedef void (__fastcall *CreateFromNodeAndPushFunction)(const Parser::Node &node, Segment* segment, void* p);

	class CommandFactoryRegister
	{
	public:
		CommandFactoryRegister(const char* id, CreateFromNodeAndPushFunction fp);
	};

	class CommandFactory
	{
		vx::sorted_vector<vx::StringID, CreateFromNodeAndPushFunction> m_functions;

	public:
		CommandFactory();
		~CommandFactory();

		void registerFunction(const char* id, CreateFromNodeAndPushFunction fp);

		void createFromNodeAndPushToSegment(const Parser::Node &node, Segment* segment, const vx::gl::ProgramPipeline* pipeline);

		static CommandFactory& get();
	};
}