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

#include "../ParserNode.h"
#include "CommandFactory.h"
#include "Segment.h"
#include "Commands/ProgramUniformCommand.h"
#include <vxLib/gl/ProgramPipeline.h>

namespace Graphics
{
	void createFromNode(const Parser::Node &node, MultiDrawElementsIndirectCountCommand* command)
	{
		auto paramsNode = node.get("params");

		u32 params[5];
		paramsNode->as(0, &params[0]);
		paramsNode->as(1, &params[1]);
		paramsNode->as(2, &params[2]);
		paramsNode->as(3, &params[3]);
		paramsNode->as(4, &params[4]);

		command->set(params[0], params[1], params[2], params[3], params[4]);
	}

	void createFromNode(const Parser::Node &node, DrawArraysIndirectCommand* command)
	{
		auto paramsNode = node.get("params");

		u32 params[2];
		paramsNode->as(0, &params[0]);
		paramsNode->as(1, &params[1]);

		command->set(params[0], params[1]);
	}

	void createFromNode(const Parser::Node &node, DrawElementsIndirectCommand* command)
	{
		auto paramsNode = node.get("params");

		u32 params[3];
		paramsNode->as(0, &params[0]);
		paramsNode->as(1, &params[1]);
		paramsNode->as(2, &params[2]);

		command->set(params[0], params[1], params[2]);
	}

	void createFromNodeAndPush(const Parser::Node &node, Segment* segment, const vx::gl::ProgramPipeline* pipeline)
	{
		auto typeNode = node.get("type");
		std::string type;
		typeNode->as(&type);

		if (strcmp(type.c_str(), "MultiDrawElementsIndirectCountCommand") == 0)
		{
			MultiDrawElementsIndirectCountCommand command;
			createFromNode(node, &command);
			segment->pushCommand(command);
		}
		else if (strcmp(type.c_str(), "ProgramUniformCommand") == 0)
		{
			auto dataTypeNode = node.get("dataType");
			auto paramsNode = node.get("params");
			auto dataNode = node.get("data");

			u32 params[3];
			paramsNode->as(0, &params[0]);
			paramsNode->as(1, &params[1]);
			paramsNode->as(2, &params[2]);

			std::string dataType;
			dataTypeNode->as(&dataType);

			ProgramUniformCommand command;

			if (strcmp(dataType.c_str(), "float") == 0)
			{
				command.setFloat((*pipeline)[(vx::gl::ShaderProgramType)params[0]], params[1], params[2]);

				if (params[2] == 1)
				{
					f32 value;
					dataNode->as((float*)&value);

					ProgramUniformData<f32> data;
					data.set(value);
					segment->pushCommand(command, data);
				}
				else if (params[2] == 3)
				{
					vx::float3 value;

					dataNode->as(0, (float*)&value[0]);
					dataNode->as(1, (float*)&value[1]);
					dataNode->as(2, (float*)&value[2]);

					ProgramUniformData<vx::float3> data;
					data.set(value);

					segment->pushCommand(command, data);
				}
				else if (params[2] == 4)
				{
					vx::float4 value;

					dataNode->as(0, (float*)&value[0]);
					dataNode->as(1, (float*)&value[1]);
					dataNode->as(2, (float*)&value[2]);
					dataNode->as(3, (float*)&value[3]);

					ProgramUniformData<vx::float4> data;
					data.set(value);

					segment->pushCommand(command, data);
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
		else if (strcmp(type.c_str(), "DrawArraysIndirectCommand") == 0)
		{
			DrawArraysIndirectCommand command;
			createFromNode(node, &command);

			segment->pushCommand(command);
		}
		else if (strcmp(type.c_str(), "DrawElementsIndirectCommand") == 0)
		{
			DrawElementsIndirectCommand command;
			createFromNode(node, &command);

			segment->pushCommand(command);
		}
		else
		{
			VX_ASSERT(false);
		}
	}

	void CommandFactory::createFromNodeAndPushToSegment(const Parser::Node &node, Segment* segment, const vx::gl::ProgramPipeline* pipeline)
	{
		createFromNodeAndPush(node, segment, pipeline);
	}
}