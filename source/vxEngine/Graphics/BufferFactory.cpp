#include "BufferFactory.h"

namespace Graphics
{
	vx::gl::Buffer BufferFactory::createVertexBuffer(U64 size, vx::gl::BufferStorageFlags::Flags flags, const U8* ptr)
	{
		vx::gl::BufferDescription desc;
		desc.bufferType = vx::gl::BufferType::Array_Buffer;
		desc.flags = flags;
		desc.immutable = 1;
		desc.size = size;
		desc.pData = ptr;

		vx::gl::Buffer buffer;
		buffer.create(desc);

		return buffer;
	}

	vx::gl::Buffer BufferFactory::createIndexBuffer(U64 size, vx::gl::BufferStorageFlags::Flags flags,  const U8* ptr)
	{
		vx::gl::BufferDescription desc;
		desc.bufferType = vx::gl::BufferType::Element_Array_Buffer;
		desc.flags = flags;
		desc.immutable = 1;
		desc.size = size;
		desc.pData = ptr;

		vx::gl::Buffer buffer;
		buffer.create(desc);

		return buffer;
	}

	vx::gl::Buffer BufferFactory::createIndirectCmdBuffer(U64 size, vx::gl::BufferStorageFlags::Flags flags, const U8* ptr)
	{
		vx::gl::BufferDescription desc;
		desc.bufferType = vx::gl::BufferType::Draw_Indirect_Buffer;
		desc.flags = flags;
		desc.immutable = 1;
		desc.size = size;
		desc.pData = ptr;

		vx::gl::Buffer buffer;
		buffer.create(desc);

		return buffer;
	}
}