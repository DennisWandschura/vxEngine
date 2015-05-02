#include "Segment.h"
#include <vxLib/gl/StateManager.h>
#include <vxLib/gl/gl.h>

namespace Graphics
{
	template<typename T>
	void handleCommand(U8* ptr, U32* offset)
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
		case 4:
			glProgramUniform4fv(command->m_program, 0, 1, dataPtr);
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
	void handleCommand<ProgramUniformCommand>(U8* ptr, U32* offset)
	{
		handleCommandImpl((ProgramUniformCommand*)ptr, offset);
		*offset += sizeof(ProgramUniformCommand);
	}

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

		for (U32 i = 0; i < m_commmands.size();)
		{
			CommandHeader header = (CommandHeader)m_commmands[i];
			U32 offset = 0;

			switch (header)
			{
			case CommandHeader::ViewportCommand:
				handleCommand<ViewportCommand>(&m_commmands[i], &offset);
				break;
			case CommandHeader::PointSizeCommand:
				handleCommand<PointSizeCommand>(&m_commmands[i], &offset);
				break;
			case CommandHeader::DrawArraysIndirectCommand:
				handleCommand<DrawArraysIndirectCommand>(&m_commmands[i], &offset);
				break;
			case CommandHeader::DrawElementsIndirectCommand:
				handleCommand<DrawElementsIndirectCommand>(&m_commmands[i], &offset);
				break;
			case CommandHeader::MultiDrawElementsIndirectCountCommand:
				handleCommand<MultiDrawElementsIndirectCountCommand>(&m_commmands[i], &offset);
				break;
			case CommandHeader::ProgramUniformCommand:
				handleCommand<ProgramUniformCommand>(&m_commmands[i], &offset);
				break;
			default:
				assert(false);
				break;
			}

			i += offset;
		}
	}
}