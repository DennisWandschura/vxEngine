#pragma once

namespace vx
{
	namespace gl
	{
		class Buffer;
		struct BufferDescription;
	}

	class StackAllocator;
}

#include <vxLib/Container/sorted_array.h>
#include <vxLib/Container/sorted_vector.h>
#include <vxLib/StringID.h>

class BufferManager
{
	vx::sorted_array<vx::StringID, vx::gl::Buffer> m_buffers;

public:
	BufferManager();
	~BufferManager();

	void initialize(U32 maxBufferCount, vx::StackAllocator* allocator);
	void shutdown();

	vx::StringID createBuffer(const char* key, const vx::gl::BufferDescription &desc);

	vx::gl::Buffer* getBuffer(const vx::StringID &sid) const;
	vx::gl::Buffer* getBuffer(const char* buffer) const;
};