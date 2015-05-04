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