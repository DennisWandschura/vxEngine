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
#include "BufferManager.h"
#include <vxLib/gl/Buffer.h>

namespace gl
{
	BufferManager::BufferManager()
	{

	}

	BufferManager::~BufferManager()
	{

	}

	void BufferManager::initialize(u32 maxBufferCount, vx::StackAllocator* allocator)
	{
		m_buffers = vx::sorted_array<vx::StringID, vx::gl::Buffer>(maxBufferCount, allocator);
	}

	void BufferManager::shutdown()
	{
		m_buffers.cleanup();
	}

	vx::StringID BufferManager::createBuffer(const char* key, const vx::gl::BufferDescription &desc)
	{
		auto sid = vx::make_sid(key);
		auto it = m_buffers.find(sid);
		VX_ASSERT(it == m_buffers.end());

		vx::gl::Buffer buffer;
		buffer.create(desc);

		it = m_buffers.insert(std::move(sid), std::move(buffer));

		if (it == m_buffers.end())
		{
			sid = 0u;
		}

		return sid;
	}

	const vx::gl::Buffer* BufferManager::getBuffer(const vx::StringID &sid) const
	{
		auto it = m_buffers.find(sid);

		const vx::gl::Buffer* ptr = nullptr;
		if (it != m_buffers.end())
		{
			ptr = &*it;
		}

		return ptr;
	}

	const vx::gl::Buffer* BufferManager::getBuffer(const char* buffer) const
	{
		auto sid = vx::make_sid(buffer);
		return getBuffer(sid);
	}

	vx::gl::Buffer* BufferManager::getBuffer(const vx::StringID &sid)
	{
		auto it = m_buffers.find(sid);

		vx::gl::Buffer* ptr = nullptr;
		if (it != m_buffers.end())
		{
			ptr = &*it;
		}

		return ptr;
	}

	vx::gl::Buffer* BufferManager::getBuffer(const char* buffer)
	{
		auto sid = vx::make_sid(buffer);
		return getBuffer(sid);
	}
}