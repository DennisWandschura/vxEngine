#pragma once

#include <vxLib/gl/Base.h>

struct DrawCommandDescription
{
	enum class Mode : U8{ TRIANGLES, POINTS };

	U32 m_count;
	U32 m_indirectBuffer;
	Mode m_mode;
	vx::gl::DataType m_dataType;

	static DrawCommandDescription create(U32 count, Mode mode, vx::gl::DataType type)
	{
		DrawCommandDescription desc;
		desc.m_count = count;
		desc.m_indirectBuffer = 0;
		desc.m_mode = mode;
		desc.m_dataType = type;

		return desc;
	}

	static DrawCommandDescription createIndirect(U32 count, U32 indirectBuffer, Mode mode, vx::gl::DataType type)
	{
		DrawCommandDescription desc;
		desc.m_count = count;
		desc.m_indirectBuffer = indirectBuffer;
		desc.m_mode = mode;
		desc.m_dataType = type;

		return desc;
	}
};