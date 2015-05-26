#include "DoubleBufferRaw.h"
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