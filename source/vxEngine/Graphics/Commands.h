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
	struct Command
	{
		virtual ~Command(){}

		virtual void execute(U32* offset) = 0;

		static void handleCommand(Command* header, U32* offset)
		{
			header->execute(offset);
		}
	};

	struct ViewportCommand : public Command
	{
		vx::uint2 m_offset;
		vx::uint2 m_size;

		void set(const vx::uint2 &offset, const vx::uint2 &size)
		{
			m_offset = offset;
			m_size = size;
		}

		void execute(U32* offset) override;
	};

	struct PointSizeCommand : public Command
	{
		F32 m_pointSize;

		void set(F32 pointSize)
		{
			m_pointSize = pointSize;
		}

		void execute(U32* offset) override;
	};

	struct DrawArraysIndirectCommand : public Command
	{
		U32 m_mode;
		U32 m_offset;

		void set(U32 mode, U32 offset = 0)
		{
			m_mode = mode;
			m_offset = offset;
		}

		void execute(U32* offset) override;
	};

	struct DrawElementsIndirectCommand : public Command
	{
		U32 m_mode;
		U32 m_type;
		U32 m_offset;

		void set(U32 mode, U32 type, U32 offset = 0)
		{
			m_mode = mode;
			m_type = type;
			m_offset = offset;
		}

		void execute(U32* offset) override;
	};

	struct MultiDrawElementsIndirectCountCommand : public Command
	{
		U32 m_mode;
		U32 m_type;
		U32 m_indirectOffset;
		U32 m_parameterBufferOffset;
		U32 m_maxdrawcount;

		void set(U32 mode, U32 type, U32 maxDrawCount, U32 indirectOffset = 0, U32 paramOffset = 0)
		{
			m_mode = mode;
			m_type = type;
			m_indirectOffset = indirectOffset;
			m_parameterBufferOffset = paramOffset;
			m_maxdrawcount = maxDrawCount;
		}

		void execute(U32* offset) override;
	};

	struct MultiDrawArraysIndirectCountCommand : public Command
	{
		U32 m_mode;
		U32 m_indirectOffset;
		U32 m_parameterBufferOffset;
		U32 m_maxdrawcount;

		void set(U32 mode, U32 maxDrawCount, U32 indirectOffset = 0, U32 paramOffset = 0)
		{
			m_mode = mode;
			m_indirectOffset = indirectOffset;
			m_parameterBufferOffset = paramOffset;
			m_maxdrawcount = maxDrawCount;
		}

		void execute(U32* offset) override;
	};

	struct ClearColorCommand : public Command
	{
		vx::float4 m_clearColor;

		void set(const vx::float4 &clearColor)
		{
			m_clearColor = clearColor;
		}

		void execute(U32* offset) override;
	};

	struct FramebufferTextureCommand : public Command
	{
		U32 m_framebufferId;
		U32 m_attachment;
		U32 m_texture;
		U32 m_level;

		void set(U32 framebufferId, U32 attachment, U32 texture, U32 level)
		{
			m_framebufferId = framebufferId;
			m_attachment = attachment;
			m_texture = texture;
			m_level = level;
		}

		void execute(U32* offset) override;
	};

	struct PolygonOffsetCommand : public Command
	{
		F32 m_factor;
		F32 m_units;

		void set(F32 factor, F32 units)
		{
			m_factor = factor;
			m_units = units;
		}

		void execute(U32* offset) override;
	};

	struct ClearCommand : public Command
	{
		U32 m_bits;

		void set(U32 bits)
		{
			m_bits = bits;
		}

		void execute(U32* offset) override;
	};

	struct ProfilerCommand : public Command
	{
		char m_name[20];
		class GpuProfiler* m_pGpuProfiler;

		void set(GpuProfiler* pGpuProfiler, const char* name)
		{
			m_pGpuProfiler = pGpuProfiler;
			strcpy_s(m_name, name);
		}

		void execute(U32* offset) override;
	};
}