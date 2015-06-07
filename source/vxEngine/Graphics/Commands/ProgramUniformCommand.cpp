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
#include <vxEngineLib/ParserNode.h>
#include "../CommandFactory.h"
#include <vxLib/gl/ProgramPipeline.h>

namespace Graphics
{
	void createFromNodeProgramUniformCommand(const Parser::Node &node, Segment* segment, void* p)
	{
		auto paramsNode = node.get("params");

		u32 params[3];
		paramsNode->as(0, &params[0]);
		paramsNode->as(1, &params[1]);
		paramsNode->as(2, &params[2]);
	//	paramsNode->as(3, &params[3]);

		ProgramUniformCommand command;
		//command.set(params[0], params[1], params[2], (vx::gl::DataType)params[3]);

		auto dataTypeNode = node.get("dataType");
		auto dataNode = node.get("data");

		std::string dataType;
		dataTypeNode->as(&dataType);

		auto pipeline = (vx::gl::ProgramPipeline*)p;

		if (strcmp(dataType.c_str(), "float") == 0)
		{
			command.set((*pipeline)[(vx::gl::ShaderProgramType)params[0]], params[1], params[2], vx::gl::DataType::Float);

			if (params[2] == 1)
			{
				f32 value;
				dataNode->as((float*)&value);

				segment->pushCommand(command, value);
			}
			else if (params[2] == 3)
			{
				vx::float3 value;

				dataNode->as(0, (float*)&value[0]);
				dataNode->as(1, (float*)&value[1]);
				dataNode->as(2, (float*)&value[2]);

				segment->pushCommand(command, value);
			}
			else if (params[2] == 4)
			{
				vx::float4 value;

				dataNode->as(0, (float*)&value[0]);
				dataNode->as(1, (float*)&value[1]);
				dataNode->as(2, (float*)&value[2]);
				dataNode->as(3, (float*)&value[3]);

				segment->pushCommand(command, value);
			}
			else
			{
				VX_ASSERT(false);
			}
		}
		else if (strcmp(dataType.c_str(), "uint") == 0)
		{
			command.set((*pipeline)[(vx::gl::ShaderProgramType)params[0]], params[1], params[2], vx::gl::DataType::Unsigned_Int);

			if (params[2] == 1)
			{
				u32 value;
				dataNode->as((u32*)&value);

				segment->pushCommand(command, value);
			}
			else if (params[2] == 3)
			{
				vx::uint3 value;

				dataNode->as(0, (u32*)&value[0]);
				dataNode->as(1, (u32*)&value[1]);
				dataNode->as(2, (u32*)&value[2]);

				segment->pushCommand(command, value);
			}
			else if (params[2] == 4)
			{
				vx::uint4 value;

				dataNode->as(0, (u32*)&value[0]);
				dataNode->as(1, (u32*)&value[1]);
				dataNode->as(2, (u32*)&value[2]);
				dataNode->as(3, (u32*)&value[3]);

				segment->pushCommand(command, value);
			}
			else
			{
				VX_ASSERT(false);
			}
		}
		else
		{
			VX_ASSERT(false);
		}
	}

	REGISTER_COMMANDFACTORY(ProgramUniformCommand, createFromNodeProgramUniformCommand);

	void ProgramUniformCommand::set(u32 program, u32 location, u32 count, vx::gl::DataType dataType)
	{
		m_program = program;
		m_dataType = dataType;
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
			VX_ASSERT(false);
			break;
		}

		*offset += sizeof(ProgramUniformCommand);
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