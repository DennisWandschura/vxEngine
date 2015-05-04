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
#include "CommandCompiler.h"

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

	void Segment::pushCommand(const U8* ptr, U32 count)
	{
		for (U32 i = 0; i < count; ++i)
		{
			m_commmands.push_back(ptr[i]);
		}
	}

	void Segment::setState(const State &state)
	{
		m_state = state;
	}

	void Segment::draw()
	{
		m_state.update();

		auto count = m_commmands.size();
		for (U32 i = 0; i < count;)
		{
			CommandHeader* header = (CommandHeader*)&m_commmands[i];
			U32 offset = 0;

			Command::handleCommand(header, &offset);

			i += offset;
		}
	}

	SegmentCompiled::SegmentCompiled()
		:m_drawFunctionFp(nullptr),
		m_buffer()
	{
	}

	SegmentCompiled::SegmentCompiled(SegmentCompiled &&rhs)
		: m_drawFunctionFp(rhs.m_drawFunctionFp),
		m_buffer(std::move(rhs.m_buffer))
	{
		rhs.m_drawFunctionFp = nullptr;
	}

	SegmentCompiled::~SegmentCompiled()
	{

	}

	SegmentCompiled& SegmentCompiled::operator = (SegmentCompiled &&rhs)
	{
		if (this != &rhs)
		{
			std::swap(m_drawFunctionFp, rhs.m_drawFunctionFp);
			std::swap(m_buffer, rhs.m_buffer);
		}

		return *this;
	}

	void SegmentCompiled::set(void* fp, std::unique_ptr<U8[]> &&buffer)
	{
		VX_ASSERT(fp);
		m_drawFunctionFp = (DrawFunctionType)fp;
		m_buffer = std::move(buffer);
	}

	void SegmentCompiled::draw()
	{
		m_drawFunctionFp();
	}
}