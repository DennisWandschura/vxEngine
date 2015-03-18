#include "BufferManager.h"
#include <vxLib/gl/gl.h>

namespace gl
{
	vx::StringID64 BufferManager::createBuffer(const vx::gl::BufferDescription &desc, const char* name)
	{
		vx::gl::Buffer buffer;
		buffer.create(desc);
		
		auto sid = vx::make_sid(name);

		VX_ASSERT(m_buffers.find(sid) == m_buffers.end(), "");

		m_buffers.insert(sid, std::move(buffer));

		return sid;
	}

	void BufferManager::bindBuffer(U32 target, U32 index, const vx::StringID64 &sid)
	{
		auto it = m_buffers.find(sid);
		glBindBufferBase(target, index, (*it).getId());
	}

	void BufferManager::bindBufferRange(U32 target, U32 index, const vx::StringID64 &sid, U32 offset, U32 size)
	{
		auto it = m_buffers.find(sid);
		glBindBufferRange(target, index, (*it).getId(), offset, size);
	}
}