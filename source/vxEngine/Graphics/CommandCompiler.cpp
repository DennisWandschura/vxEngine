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
#include "CommandCompiler.h"
#include "Commands.h"


namespace
{
	template<typename T>
	void compileCommand(Graphics::CommandHeader* ptr, U32* offset, std::string* data)
	{
		compileCommandImpl((T*)ptr, data);
		*offset += sizeof(T);
	}

	void compileCommandImpl(const Graphics::ViewportCommand* command, std::string* data)
	{
		*data += std::string("vx::gl::StateManager::setViewport(") 
			+ std::to_string(command->m_offset.x)
			+ std::string(", ")
			+ std::to_string(command->m_offset.y)
			+ std::string(", ")
			+ std::to_string(command->m_size.x)
			+ std::string(", ")
			+ std::to_string(command->m_size.y)
			+ std::string(");\n");
	}

	void compileCommandImpl(const Graphics::PointSizeCommand* command, std::string* data)
	{
		*data += std::string("glPointSize(") + std::to_string(command->m_pointSize) + std::string(");\n");
	}

	void compileCommandImpl(const Graphics::DrawArraysIndirectCommand* command, std::string* data)
	{
		*data += std::string("glDrawArraysIndirect(")
			+ std::to_string(command->m_mode)
			+ std::string(", (void*)")
			+ std::to_string(command->m_offset)
			+ std::string(");\n");
	}

	void compileCommandImpl(const Graphics::DrawElementsIndirectCommand* command, std::string* data)
	{
		*data += std::string("glDrawElementsIndirect(") 
			+ std::to_string(command->m_mode)
			+ std::string(", ") 
			+ std::to_string(command->m_type)
			+ std::string(", (void*)")
			+ std::to_string(command->m_offset)
			+ std::string(");\n");
	}

	void compileCommandImpl(const Graphics::MultiDrawElementsIndirectCountCommand* command, std::string* data)
	{
		*data += std::string("glMultiDrawElementsIndirectCountARB(")
			+ std::to_string(command->m_mode)
			+ std::string(", ")
			+ std::to_string(command->m_type)
			+ std::string(", ")
			+ std::to_string(command->m_indirectOffset)
			+ std::string(", ")
			+ std::to_string(command->m_parameterBufferOffset)
			+ std::string(", ")
			+ std::to_string(command->m_maxdrawcount)
			+ std::string(", ")
			+ std::to_string(sizeof(vx::gl::DrawElementsIndirectCommand))
			+ std::string(");\n");
	}

	void programUniformFloat(const Graphics::ProgramUniformCommand* command, U32* offset, std::string* data, U8* dataBuffer, U32* bufferOffset)
	{
		auto dataSizeInBytes = command->m_count * sizeof(F32);
		const F32* dataPtr = (F32*)(command + 1);

		switch (command->m_count)
		{
		case 1:
		{
			*data += std::string("glProgramUniform1f(")
				+ std::to_string(command->m_program )
				+ std::string(", ")
				+ std::to_string(command->m_location)
				+ std::string(", ")
				+ std::to_string(*dataPtr)
				+ std::string(");\n");
		}break;
		case 4:
		{
			auto srcPtr = dataBuffer + *bufferOffset;

			*data += std::string("glProgramUniform4fv(")
				+ std::to_string(command->m_program)
				+ std::string(", ")
				+ std::to_string(command->m_location)
				+ std::string(", 1, (const F32*)")
				+ std::to_string((U64)srcPtr)
				+ std::string(");\n");

			*bufferOffset += dataSizeInBytes;

		}break;
		default:
			assert(false);
			break;
		}

		*offset += (command->m_count * sizeof(F32));
	}

	void compileCommandImpl(const Graphics::ProgramUniformCommand* command, U32* offset, std::string* data, U8* dataBuffer, U32* bufferOffset)
	{
		switch (command->m_dataType)
		{
		case vx::gl::DataType::Float:
			programUniformFloat(command, offset, data, dataBuffer, bufferOffset);
			break;
		default:
			assert(false);
			break;
		}
	}

	void compileCommand(Graphics::CommandHeader* ptr, U32* offset, std::string* data, U8* dataBuffer, U32* bufferOffset)
	{
		compileCommandImpl((Graphics::ProgramUniformCommand*)ptr, offset, data, dataBuffer, bufferOffset);
		*offset += sizeof(Graphics::ProgramUniformCommand);
	}

	template<typename T>
	void getNextCommand(U32* offset)
	{
		*offset += sizeof(T);
	}

	void getNextCommand(Graphics::ProgramUniformCommand* command, U32* offset)
	{
		*offset += sizeof(Graphics::ProgramUniformCommand);

		U32 dataSize = 0;
		switch (command->m_dataType)
		{
		case vx::gl::DataType::Float:
			dataSize = sizeof(F32);
			break;
		default:
			assert(false);
			break;
		}

		*offset += (command->m_count * dataSize);
	}
}

namespace Graphics
{
	void CommandCompiler::getNextCommand(CommandHeader* ptr, U32* offset)
	{
		switch (*ptr)
		{
		case CommandHeader::ViewportCommand:
			::getNextCommand<ViewportCommand>(offset);
			break;
		case CommandHeader::PointSizeCommand:
			::getNextCommand<PointSizeCommand>(offset);
			break;
		case CommandHeader::DrawArraysIndirectCommand:
			::getNextCommand<DrawArraysIndirectCommand>(offset);
			break;
		case CommandHeader::DrawElementsIndirectCommand:
			::getNextCommand<DrawElementsIndirectCommand>(offset);
			break;
		case CommandHeader::MultiDrawElementsIndirectCountCommand:
			::getNextCommand<MultiDrawElementsIndirectCountCommand>(offset);
			break;
		case CommandHeader::ProgramUniformCommand:
			::getNextCommand((ProgramUniformCommand*)ptr, offset);
			break;
		default:
			assert(false);
			break;
		}
	}

	void CommandCompiler::compileCommand(CommandHeader* ptr, U32* offset, std::string* srcFile, U8* dataBuffer, U32* bufferOffset)
	{
		switch (*ptr)
		{
		case CommandHeader::ViewportCommand:
			::compileCommand<ViewportCommand>(ptr, offset, srcFile);
			break;
		case CommandHeader::PointSizeCommand:
			::compileCommand<PointSizeCommand>(ptr, offset, srcFile);
			break;
		case CommandHeader::DrawArraysIndirectCommand:
			::compileCommand<DrawArraysIndirectCommand>(ptr, offset, srcFile);
			break;
		case CommandHeader::DrawElementsIndirectCommand:
			::compileCommand<DrawElementsIndirectCommand>(ptr, offset, srcFile);
			break;
		case CommandHeader::MultiDrawElementsIndirectCountCommand:
			::compileCommand<MultiDrawElementsIndirectCountCommand>(ptr, offset, srcFile);
			break;
		case CommandHeader::ProgramUniformCommand:
			::compileCommand(ptr, offset, srcFile, dataBuffer, bufferOffset);
			break;
		default:
			assert(false);
			break;
		}
	}
}