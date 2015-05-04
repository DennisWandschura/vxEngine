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

	vx::gl::Buffer BufferFactory::createParameterBuffer(U64 size, vx::gl::BufferStorageFlags::Flags flags, const U8* ptr)
	{
		vx::gl::BufferDescription desc;
		desc.bufferType = vx::gl::BufferType::Parameter_Buffer;
		desc.flags = flags;
		desc.immutable = 1;
		desc.size = size;
		desc.pData = ptr;

		vx::gl::Buffer buffer;
		buffer.create(desc);

		return buffer;
	}
}