#include <vxLib/math/Vector.h>
#include <vector>
#include <vxLib/gl/Base.h>
#include "State.h"

namespace Graphics
{
	enum class CommandHeader : U32
	{
		ViewportCommand,
		PointSizeCommand,
		DrawArraysIndirectCommand,
		DrawElementsIndirectCommand,
		MultiDrawElementsIndirectCountCommand,
		ProgramUniformCommand
	};

	struct ViewportCommand
	{
		CommandHeader m_header;
		vx::uint2 m_offset;
		vx::uint2 m_size;

		void set(const vx::uint2 &offset, const vx::uint2 &size)
		{
			m_header = CommandHeader::ViewportCommand;
			m_offset = offset;
			m_size = size;
		}
	};

	struct PointSizeCommand
	{
		CommandHeader m_header;
		F32 m_pointSize;

		void set(F32 pointSize)
		{
			m_header = CommandHeader::PointSizeCommand;
			m_pointSize = pointSize;
		}
	};

	struct ProgramUniformCommand
	{
		CommandHeader m_header;
		U32 m_program;
		vx::gl::DataType m_dataType;
		U8 m_padding;
		U16 m_count;

		void setFloat4(U32 program)
		{
			m_header = CommandHeader::ProgramUniformCommand;
			m_program = program;
			m_dataType = vx::gl::DataType::Float;
			m_count = 4;
		}
	};

	template<typename T>
	struct ProgramUniformData
	{
		enum { Count = (sizeof(T) + sizeof(U32) - 1) / sizeof(U32) };

		U32 u[Count];

		ProgramUniformData() : u()
		{
		}

		void set(const T &data)
		{
			memcpy(u, &data, sizeof(T));
		}

		U32& operator[](U32 i)
		{
			return u[i];
		}

		const U32& operator[](U32 i) const
		{
			return u[i];
		}
	};

	struct DrawArraysIndirectCommand
	{
		CommandHeader m_header;
		U32 m_mode;

		void set(U32 mode)
		{
			m_header = CommandHeader::DrawArraysIndirectCommand;
			m_mode = mode;
		}
	};

	struct DrawElementsIndirectCommand
	{
		CommandHeader m_header;
		U32 m_mode;
		U32 m_type;

		void set(U32 mode, U32 type)
		{
			m_header = CommandHeader::DrawElementsIndirectCommand;
			m_mode = mode;
			m_type = type;
		}
	};

	struct MultiDrawElementsIndirectCountCommand
	{
		CommandHeader m_header;
		U32 m_mode;
		U32 m_type;
		U32 m_indirectOffset;
		U32 m_parameterBufferOffset;
		U32 m_maxdrawcount;

		void set(U32 mode, U32 type, U32 maxDrawCount, U32 indirectOffset = 0, U32 paramOffset = 0)
		{
			m_header = CommandHeader::MultiDrawElementsIndirectCountCommand;
			m_mode = mode;
			m_type = type;
			m_indirectOffset = indirectOffset;
			m_parameterBufferOffset = paramOffset;
			m_maxdrawcount = maxDrawCount;
		}
	};

	class Segment
	{
		std::vector<U32> m_commmands;
		State m_state;

	public:
		Segment();
		~Segment();

		void setState(const State &state);

		template < typename T >
		void pushCommand(const T &command)
		{
			U32* ptr = (U32*)&command;
			auto count = sizeof(T) / sizeof(U32);

			for (U32 i = 0; i < count; ++i)
			{
				m_commmands.push_back(ptr[i]);
			}
		}

		template < typename T >
		void pushCommand(const ProgramUniformCommand &command, const ProgramUniformData<T> &data)
		{
			U32* ptr = (U32*)&command;
			auto count = sizeof(ProgramUniformCommand) / sizeof(U32);

			for (U32 i = 0; i < count; ++i)
			{
				m_commmands.push_back(ptr[i]);
			}

			for (U32 i = 0; i < ProgramUniformData<T>::Count; ++i)
			{
				m_commmands.push_back(data[i]);
			}
		}

		void draw();
	};
}