#include "Commands.h"
#include <vxLib/gl/StateManager.h>
#include <vxLib/gl/gl.h>

namespace Graphics
{
	template<typename T>
	void handleCommandFun(CommandHeader* ptr, U32* offset)
	{
		handleCommandImpl((T*)ptr);
		*offset += sizeof(T);
	}

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

	void handleCommandImpl(const ProgramUniformCommand* command, U32* offset)
	{
		switch (command->m_dataType)
		{
		case vx::gl::DataType::Float:
			programUniformFloat(command, offset);
			break;
		default:
			assert(false);
			break;
		}
	}

	template<>
	void handleCommandFun<ProgramUniformCommand>(CommandHeader* ptr, U32* offset)
	{
		handleCommandImpl((ProgramUniformCommand*)ptr, offset);
		*offset += sizeof(ProgramUniformCommand);
	}

	void Command::handleCommand(CommandHeader* header, U32* offset)
	{
		switch (*header)
		{
		case CommandHeader::ViewportCommand:
			handleCommandFun<ViewportCommand>(header, offset);
			break;
		case CommandHeader::PointSizeCommand:
			handleCommandFun<PointSizeCommand>(header, offset);
			break;
		case CommandHeader::DrawArraysIndirectCommand:
			handleCommandFun<DrawArraysIndirectCommand>(header, offset);
			break;
		case CommandHeader::DrawElementsIndirectCommand:
			handleCommandFun<DrawElementsIndirectCommand>(header, offset);
			break;
		case CommandHeader::MultiDrawElementsIndirectCountCommand:
			handleCommandFun<MultiDrawElementsIndirectCountCommand>(header, offset);
			break;
		case CommandHeader::ProgramUniformCommand:
			handleCommandFun<ProgramUniformCommand>(header, offset);
			break;
		default:
			assert(false);
			break;
		}
	}
}