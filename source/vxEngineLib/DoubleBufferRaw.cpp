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

#include <vxEngineLib/DoubleBufferRaw.h>
#include <algorithm>

DoubleBufferRaw::DoubleBufferRaw()
	:m_frontBuffer(nullptr),
	m_backBuffer(nullptr),
	m_frontSize(0),
	m_backSize(0),
	m_capacity(0)
{
}

DoubleBufferRaw::DoubleBufferRaw(vx::StackAllocator* allocator, u32 capacity)
	:m_frontBuffer(nullptr),
	m_backBuffer(nullptr),
	m_frontSize(0),
	m_backSize(0),
	m_capacity(0)
{
	auto front = allocator->allocate(capacity, 64);
	auto back = allocator->allocate(capacity, 64);

	m_frontBuffer = front;
	m_backBuffer = back;

	m_capacity = capacity;
}

bool DoubleBufferRaw::memcpy(const u8* data, u32 size)
{
	auto newSize = m_frontSize + size;
	if (newSize >= m_capacity)
		return false;

	::memcpy(m_frontBuffer + m_frontSize, data, size);
	m_frontSize = newSize;

	return true;
}

void DoubleBufferRaw::swapBuffers()
{
	std::swap(m_frontBuffer, m_backBuffer);
	std::swap(m_frontSize, m_backSize);
	m_frontSize = 0;
}

u8* DoubleBufferRaw::getBackBuffer()
{
	return m_backBuffer;
}

u32 DoubleBufferRaw::getBackBufferSize() const
{
	return m_backSize;
}