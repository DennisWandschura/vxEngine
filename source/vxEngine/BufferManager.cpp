#include "BufferManager.h"
#include <vxLib/gl/Buffer.h>

BufferManager::BufferManager()
{

}

BufferManager::~BufferManager()
{

}

void BufferManager::initialize(U32 maxBufferCount, vx::StackAllocator* allocator)
{
	m_buffers = vx::sorted_array<vx::StringID, vx::gl::Buffer>(maxBufferCount, allocator);
}

void BufferManager::shutdown()
{
	m_buffers.cleanup();
	//m_buffers.clear();
}

vx::StringID BufferManager::createBuffer(const char* key, const vx::gl::BufferDescription &desc)
{
	vx::gl::Buffer buffer;
	buffer.create(desc);

	auto sid = vx::make_sid(key);
	auto it = m_buffers.insert(sid, std::move(buffer));

	if (it == m_buffers.end())
	{
		sid = 0u;
	}

	return sid;
}

vx::gl::Buffer* BufferManager::getBuffer(const vx::StringID &sid) const
{
	auto it = m_buffers.find(sid);

	vx::gl::Buffer* ptr = nullptr;
	if (it != m_buffers.end())
	{
		ptr = &*it;
	}

	return ptr;
}

vx::gl::Buffer* BufferManager::getBuffer(const char* buffer) const
{
	auto sid = vx::make_sid(buffer);
	return getBuffer(sid);
}