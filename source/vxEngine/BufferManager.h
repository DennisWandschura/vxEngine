#pragma once

#include <vxLib/Container/sorted_array.h>
#include <vxLib/StringID.h>
#include <vxLib/gl/Buffer.h>

namespace gl
{
	class BufferManager
	{

		vx::sorted_array<vx::StringID64, vx::gl::Buffer> m_buffers;

	public:
		vx::StringID64 createBuffer(const vx::gl::BufferDescription &desc, const char* name);

		void bindBuffer(U32 target, U32 index, const vx::StringID64 &sid);
		void bindBufferRange(U32 target, U32 index, const vx::StringID64 &sid, U32 offset, U32 size);
	};
}