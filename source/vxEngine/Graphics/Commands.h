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
	class Segment;

	struct Command
	{
		virtual ~Command(){}

		virtual void execute(u32* offset) = 0;

		virtual void pushToSegment(Segment* segment) = 0;

		static void handleCommand(Command* header, u32* offset)
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

		void execute(u32* offset) override;
		void pushToSegment(Segment* segment) override;
	};

	struct PointSizeCommand : public Command
	{
		f32 m_pointSize;

		void set(f32 pointSize)
		{
			m_pointSize = pointSize;
		}

		void execute(u32* offset) override;
		void pushToSegment(Segment* segment) override;
	};

	struct DrawArraysIndirectCommand : public Command
	{
		u32 m_mode;
		u32 m_offset;

		void set(u32 mode, u32 offset = 0)
		{
			m_mode = mode;
			m_offset = offset;
		}

		void execute(u32* offset) override;
		void pushToSegment(Segment* segment) override;
	};

	struct DrawElementsIndirectCommand : public Command
	{
		u32 m_mode;
		u32 m_type;
		u32 m_offset;

		void set(u32 mode, u32 type, u32 offset = 0)
		{
			m_mode = mode;
			m_type = type;
			m_offset = offset;
		}

		void execute(u32* offset) override;
		void pushToSegment(Segment* segment) override;
	};

	struct MultiDrawElementsIndirectCountCommand : public Command
	{
		u32 m_mode;
		u32 m_type;
		u32 m_indirectOffset;
		u32 m_parameterBufferOffset;
		u32 m_maxdrawcount;

		void set(u32 mode, u32 type, u32 maxDrawCount, u32 indirectOffset = 0, u32 paramOffset = 0)
		{
			m_mode = mode;
			m_type = type;
			m_indirectOffset = indirectOffset;
			m_parameterBufferOffset = paramOffset;
			m_maxdrawcount = maxDrawCount;
		}

		void execute(u32* offset) override;
		void pushToSegment(Segment* segment) override;
	};

	struct MultiDrawArraysIndirectCountCommand : public Command
	{
		u32 m_mode;
		u32 m_indirectOffset;
		u32 m_parameterBufferOffset;
		u32 m_maxdrawcount;

		void set(u32 mode, u32 maxDrawCount, u32 indirectOffset = 0, u32 paramOffset = 0)
		{
			m_mode = mode;
			m_indirectOffset = indirectOffset;
			m_parameterBufferOffset = paramOffset;
			m_maxdrawcount = maxDrawCount;
		}

		void execute(u32* offset) override;
		void pushToSegment(Segment* segment) override;
	};

	struct ClearColorCommand : public Command
	{
		vx::float4 m_clearColor;

		void set(const vx::float4 &clearColor)
		{
			m_clearColor = clearColor;
		}

		void execute(u32* offset) override;
		void pushToSegment(Segment* segment) override;
	};

	struct FramebufferTextureCommand : public Command
	{
		u32 m_framebufferId;
		u32 m_attachment;
		u32 m_texture;
		u32 m_level;

		void set(u32 framebufferId, u32 attachment, u32 texture, u32 level)
		{
			m_framebufferId = framebufferId;
			m_attachment = attachment;
			m_texture = texture;
			m_level = level;
		}

		void execute(u32* offset) override;
		void pushToSegment(Segment* segment) override;
	};

	struct PolygonOffsetCommand : public Command
	{
		f32 m_factor;
		f32 m_units;

		void set(f32 factor, f32 units)
		{
			m_factor = factor;
			m_units = units;
		}

		void execute(u32* offset) override;
		void pushToSegment(Segment* segment) override;
	};

	struct ClearCommand : public Command
	{
		u32 m_bits;

		void set(u32 bits)
		{
			m_bits = bits;
		}

		void execute(u32* offset) override;
		void pushToSegment(Segment* segment) override;
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

		void execute(u32* offset) override;
		void pushToSegment(Segment* segment) override;
	};

	struct BarrierCommand : public Command
	{
		u32 m_barrierBits;

		void set(u32 barrierBits)
		{
			m_barrierBits = barrierBits;
		}

		void execute(u32* offset) override;
		void pushToSegment(Segment* segment) override;
	};
}