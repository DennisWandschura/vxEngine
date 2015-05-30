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
#include "Segment.h"
#include "Commands.h"
#include "Commands/ProgramUniformCommand.h"

namespace Graphics
{
	Segment::Segment()
		:m_commmands(),
		m_state()
	{

	}

	Segment::~Segment()
	{

	}

	void Segment::pushCommand(const u8* ptr, u32 count)
	{
		for (u32 i = 0; i < count; ++i)
		{
			m_commmands.push_back(ptr[i]);
		}
	}

	void Segment::setState(const State &state)
	{
		m_state = state;
	}

	void Segment::pushCommand(const ProgramUniformCommand &command, const u8* data)
	{
		static_assert(__alignof(ProgramUniformCommand) == 8, "");

		u8* ptr = (u8*)&command;
		pushCommand(ptr, sizeof(ProgramUniformCommand));

		u8 bytePerItem = 4;
		if (command.m_dataType == vx::gl::DataType::Float)
			bytePerItem = 4;
		else if (command.m_dataType == vx::gl::DataType::Int)
			bytePerItem = 4;
		else if (command.m_dataType == vx::gl::DataType::Unsigned_Int)
			bytePerItem = 4;

		auto dataSize = command.m_count * bytePerItem;
		pushCommand(data, dataSize);
	}

	void Segment::draw()
	{
		m_state.update();

		auto count = m_commmands.size();
		for (u32 i = 0; i < count;)
		{
			Command* header = (Command*)&m_commmands[i];
			u32 offset = 0;

			Command::handleCommand(header, &offset);
			VX_ASSERT(offset != 0);

			i += offset;
		}
	}

	bool Segment::isValid() const
	{
		return m_state.isValid() && !m_commmands.empty();
	}
}