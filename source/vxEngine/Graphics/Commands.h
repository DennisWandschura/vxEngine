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
#pragma once

#include <vxLib/gl/Base.h>
#include <vxLib/math/Vector.h>

namespace Graphics
{
	enum class CommandHeader : U32
	{
		ViewportCommand,
		PointSizeCommand,
		DrawArraysIndirectCommand,
		DrawElementsIndirectCommand,
		MultiDrawElementsIndirectCountCommand,
		MultiDrawArraysIndirectCountCommand,
		ProgramUniformCommand,
		ClearColorCommand,
		FramebufferTextureCommand,
		PolygonOffsetCommand,
		ClearCommand,
		ProfilerCommand
	};

	struct Command
	{
		static void handleCommand(CommandHeader* header, U32* offset);
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
		U32 m_location;

		void setUInt(U32 program, U32 location)
		{
			m_header = CommandHeader::ProgramUniformCommand;
			m_program = program;
			m_dataType = vx::gl::DataType::Unsigned_Int;
			m_count = 1;
			m_location = location;
		}

		void setFloat(U32 program, U32 location)
		{
			m_header = CommandHeader::ProgramUniformCommand;
			m_program = program;
			m_dataType = vx::gl::DataType::Float;
			m_count = 1;
			m_location = location;
		}

		void setFloat4(U32 program, U32 location)
		{
			m_header = CommandHeader::ProgramUniformCommand;
			m_program = program;
			m_dataType = vx::gl::DataType::Float;
			m_count = 4;
			m_location = location;
		}
	};

	template<typename T>
	struct ProgramUniformData
	{
		enum { Count = sizeof(T) };

		U8 u[Count];

		ProgramUniformData() : u()
		{
		}

		void set(const T &data)
		{
			memcpy(u, &data, sizeof(T));
		}

		U8& operator[](U32 i)
		{
			return u[i];
		}

		const U8& operator[](U32 i) const
		{
			return u[i];
		}
	};

	struct DrawArraysIndirectCommand
	{
		CommandHeader m_header;
		U32 m_mode;
		U32 m_offset;

		void set(U32 mode, U32 offset = 0)
		{
			m_header = CommandHeader::DrawArraysIndirectCommand;
			m_mode = mode;
			m_offset = offset;
		}
	};

	struct DrawElementsIndirectCommand
	{
		CommandHeader m_header;
		U32 m_mode;
		U32 m_type;
		U32 m_offset;

		void set(U32 mode, U32 type, U32 offset = 0)
		{
			m_header = CommandHeader::DrawElementsIndirectCommand;
			m_mode = mode;
			m_type = type;
			m_offset = offset;
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

	struct MultiDrawArraysIndirectCountCommand
	{
		CommandHeader m_header;
		U32 m_mode;
		U32 m_indirectOffset;
		U32 m_parameterBufferOffset;
		U32 m_maxdrawcount;

		void set(U32 mode, U32 maxDrawCount, U32 indirectOffset = 0, U32 paramOffset = 0)
		{
			m_header = CommandHeader::MultiDrawArraysIndirectCountCommand;
			m_mode = mode;
			m_indirectOffset = indirectOffset;
			m_parameterBufferOffset = paramOffset;
			m_maxdrawcount = maxDrawCount;
		}
	};

	struct ClearColorCommand
	{
		CommandHeader m_header;
		vx::float4 m_clearColor;

		void set(const vx::float4 &clearColor)
		{
			m_header = CommandHeader::ClearColorCommand;
			m_clearColor = clearColor;
		}
	};

	struct FramebufferTextureCommand
	{
		CommandHeader m_header;
		U32 m_framebufferId;
		U32 m_attachment;
		U32 m_texture;
		U32 m_level;

		void set(U32 framebufferId, U32 attachment, U32 texture, U32 level)
		{
			m_header = CommandHeader::FramebufferTextureCommand;
			m_framebufferId = framebufferId;
			m_attachment = attachment;
			m_texture = texture;
			m_level = level;
		}
	};

	struct PolygonOffsetCommand
	{
		CommandHeader m_header;
		F32 m_factor;
		F32 m_units;

		void set(F32 factor, F32 units)
		{
			m_header = CommandHeader::PolygonOffsetCommand;
			m_factor = factor;
			m_units = units;
		}
	};

	struct ClearCommand
	{
		CommandHeader m_header;
		U32 m_bits;

		void set(U32 bits)
		{
			m_header = CommandHeader::ClearCommand;
			m_bits = bits;
		}
	};

	struct ProfilerCommand
	{
		CommandHeader m_header;
		char m_name[20];
		class GpuProfiler* m_pGpuProfiler;

		void set(GpuProfiler* pGpuProfiler, const char* name)
		{
			m_header = CommandHeader::ProfilerCommand;
			m_pGpuProfiler = pGpuProfiler;
			strcpy_s(m_name, name);
		}
	};
}