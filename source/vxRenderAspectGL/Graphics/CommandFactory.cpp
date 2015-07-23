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

#include "CommandFactory.h"
#include <vxEngineLib/ParserNode.h>
#include <vxLib/Singleton.h>

namespace Graphics
{
	typedef vx::GlobalSingleton<CommandFactory, vx::AssertCheck, vx::CreationImplicit> SingletonCommandFactory;

	CommandFactoryRegister::CommandFactoryRegister(const char* id, CreateFromNodeAndPushFunction fp)
	{
		CommandFactory::get().registerFunction(id, fp);
	}

	CommandFactory::CommandFactory()
		:m_functions()
	{

	}

	CommandFactory::~CommandFactory()
	{

	}

	void CommandFactory::registerFunction(const char* id, CreateFromNodeAndPushFunction fp)
	{
		m_functions.insert(vx::make_sid(id), fp);
	}

	void CommandFactory::createFromNodeAndPushToSegment(const Parser::Node &node, Segment* segment, const vx::gl::ProgramPipeline* pipeline)
	{
		auto typeNode = node.get("type");
		std::string type;
		typeNode->as(&type);

		auto it = m_functions.find(vx::make_sid(type.c_str()));
		if (it != m_functions.end())
		{
			auto fp = *it;

			fp(node, segment, (void*)pipeline);
		}
	}

	CommandFactory& CommandFactory::get()
	{
		return SingletonCommandFactory::get();
	}
}