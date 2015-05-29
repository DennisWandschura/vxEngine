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

#include "ProgramUniformCommand.h"
#include <vxLib/gl/gl.h>
#include "../Segment.h"

namespace Graphics
{
	void ProgramUniformCommand::setUInt(u32 program, u32 location, u32 count)
	{
		m_program = program;
		m_dataType = vx::gl::DataType::Unsigned_Int;
		m_count = count;
		m_location = location;
	}

	void ProgramUniformCommand::setFloat(u32 program, u32 location, u32 count)
	{
		m_program = program;
		m_dataType = vx::gl::DataType::Float;
		m_count = count;
		m_location = location;
	}

	void ProgramUniformCommand::execute(u32* offset)
	{
		switch (m_dataType)
		{
		case vx::gl::DataType::Float:
			programUniformFloat(offset);
			break;
		case vx::gl::DataType::Unsigned_Int:
			programUniformUInt(offset);
			break;
		default:
			assert(false);
			break;
		}

		*offset += sizeof(ProgramUniformCommand);
	}

	void ProgramUniformCommand::pushToSegment(Segment* segment)
	{
	}

	void ProgramUniformCommand::programUniformFloat(u32* offset)
	{
		const f32* dataPtr = (f32*)(this + 1);
		switch (m_count)
		{
		case 1:
			glProgramUniform1f(m_program, m_location, *dataPtr);
			break;
		case 4:
			glProgramUniform4fv(m_program, m_location, 1, dataPtr);
			break;
		default:
			assert(false);
			break;
		}

		*offset += (m_count * sizeof(f32));
	}

	void ProgramUniformCommand::programUniformUInt(u32* offset)
	{
		const u32* dataPtr = (u32*)(this + 1);
		switch (m_count)
		{
		case 1:
			glProgramUniform1ui(m_program, m_location, *dataPtr);
			break;
		default:
			assert(false);
			break;
		}

		*offset += (m_count * sizeof(u32));
	}
}