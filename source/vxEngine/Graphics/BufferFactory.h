#pragma once

#include <vxLib/gl/Buffer.h>

namespace vx
{
	namespace gl
	{
		class Buffer;
	}
}

namespace Graphics
{
	class BufferFactory
	{
	public:
		static vx::gl::Buffer createVertexBuffer(U64 size, vx::gl::BufferStorageFlags::Flags flags, const U8* ptr = nullptr);
		static vx::gl::Buffer createIndexBuffer(U64 size, vx::gl::BufferStorageFlags::Flags flags, const U8* ptr = nullptr);
		static vx::gl::Buffer createIndirectCmdBuffer(U64 size, vx::gl::BufferStorageFlags::Flags flags, const U8* ptr);
	};
}