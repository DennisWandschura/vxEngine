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
#include "Commands.h"
#include <vxLib/gl/StateManager.h>
#include <vxLib/gl/gl.h>

namespace Graphics
{
	typedef void(*HandleCommandProc)(CommandHeader*, U32*);

	void handleCommandImpl(const ViewportCommand* command)
	{
		vx::gl::StateManager::setViewport(command->m_offset.x, command->m_offset.y, command->m_size.x, command->m_size.y);
	}

	void handleCommandImpl(const PointSizeCommand* command)
	{
		glPointSize(command->m_pointSize);
	}

	void handleCommandImpl(const DrawArraysIndirectCommand* command)
	{
		glDrawArraysIndirect(command->m_mode, (void*)command->m_offset);
	}

	void handleCommandImpl(const DrawElementsIndirectCommand* command)
	{
		glDrawElementsIndirect(command->m_mode, command->m_type, (void*)command->m_offset);
	}

	void programUniformFloat(const ProgramUniformCommand* command, U32* offset)
	{
		const U32* dataPtr = (U32*)(command + 1);
		switch (command->m_count)
		{
		case 1:
			glProgramUniform1ui(command->m_program, command->m_location, *dataPtr);
			break;
		default:
			assert(false);
			break;
		}

		*offset += (command->m_count * sizeof(U32));
	}

	void programUniformUInt(const ProgramUniformCommand* command, U32* offset)
	{
		const F32* dataPtr = (F32*)(command + 1);
		switch (command->m_count)
		{
		case 1:
			glProgramUniform1f(command->m_program, command->m_location, *dataPtr);
			break;
		case 4:
			glProgramUniform4fv(command->m_program, command->m_location, 1, dataPtr);
			break;
		default:
			assert(false);
			break;
		}

		*offset += (command->m_count * sizeof(F32));
	}

	void handleCommandImpl(const MultiDrawElementsIndirectCountCommand* command)
	{
		glMultiDrawElementsIndirectCountARB(command->m_mode, command->m_type, command->m_indirectOffset, command->m_parameterBufferOffset, command->m_maxdrawcount, sizeof(vx::gl::DrawElementsIndirectCommand));
	}

	void handleCommandImpl(const MultiDrawArraysIndirectCountCommand* command)
	{
		glMultiDrawArraysIndirectCountARB(command->m_mode, command->m_indirectOffset, command->m_parameterBufferOffset, command->m_maxdrawcount, sizeof(vx::gl::DrawArraysIndirectCommand));
	}

	void handleCommandImpl(const ClearColorCommand* command)
	{
		vx::gl::StateManager::setClearColor(command->m_clearColor.x, command->m_clearColor.y, command->m_clearColor.z, command->m_clearColor.w);
	}

	void handleCommandImpl(const ProgramUniformCommand* command, U32* offset)
	{
		switch (command->m_dataType)
		{
		case vx::gl::DataType::Float:
			programUniformFloat(command, offset);
			break;
		case vx::gl::DataType::Unsigned_Int:
			programUniformUInt(command, offset);
			break;
		default:
			assert(false);
			break;
		}
	}

	template<typename T>
	void handleCommandFun(CommandHeader* ptr, U32* offset)
	{
		handleCommandImpl((T*)ptr);
		*offset += sizeof(T);
	}

	template<>
	void handleCommandFun<ProgramUniformCommand>(CommandHeader* ptr, U32* offset)
	{
		handleCommandImpl((ProgramUniformCommand*)ptr, offset);
		*offset += sizeof(ProgramUniformCommand);
	}

	void handleCommandImpl(const FramebufferTextureCommand* command)
	{
		glNamedFramebufferTexture(command->m_framebufferId, command->m_attachment, command->m_texture, command->m_level);
	}

	void handleCommandImpl(const PolygonOffsetCommand* command)
	{
		glPolygonOffset(command->m_factor, command->m_units);
	}

	HandleCommandProc g_functionTable[] =
	{
		&handleCommandFun<ViewportCommand>,
		&handleCommandFun<PointSizeCommand>,
		&handleCommandFun<DrawArraysIndirectCommand>,
		&handleCommandFun<DrawElementsIndirectCommand>,
		&handleCommandFun<MultiDrawElementsIndirectCountCommand>,
		&handleCommandFun<MultiDrawArraysIndirectCountCommand>,
		&handleCommandFun<ProgramUniformCommand>,
		&handleCommandFun<ClearColorCommand>,
		&handleCommandFun<FramebufferTextureCommand>,
		&handleCommandFun<PolygonOffsetCommand>
	};

	void Command::handleCommand(CommandHeader* header, U32* offset)
	{
		g_functionTable[(U32)*header](header, offset);
	}
}