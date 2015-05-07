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

#include <vxLib/types.h>

class Memory
{
	u8* m_pMemory{nullptr};
	size_t m_size;
	size_t m_alignment;

public:
	Memory(){}

	Memory(size_t size, size_t alignment)
		:m_size(size),
		m_alignment(alignment)
	{
		m_pMemory = (u8*)_aligned_malloc(size, alignment);
		assert(m_pMemory);
	}

	Memory(const Memory&) = delete;

	Memory(Memory &&rhs)
		:m_pMemory(rhs.m_pMemory),
		m_size(rhs.m_size),
		m_alignment(rhs.m_alignment)
	{
		rhs.m_pMemory = nullptr;
		rhs.m_size = 0;
	}

	~Memory()
	{
		clear();
	}

	Memory& operator=(const Memory&) = delete;

	Memory& operator=(Memory &&rhs)
	{
		if (this != &rhs)
		{
			this->swap(rhs);
		}
		return *this;
	}

	void swap(Memory &other)
	{
		std::swap(m_pMemory, other.m_pMemory);
		std::swap(m_size, other.m_size);
		std::swap(m_alignment, other.m_alignment);
	}

	void clear()
	{
		if (m_pMemory)
		{
			_aligned_free(m_pMemory);
			m_pMemory = nullptr;
		}
	}

	u8* get()
	{
		return m_pMemory;
	}

	const u8* get() const
	{
		return m_pMemory;
	}

	size_t size() const
	{
		return m_size;
	}

	size_t alignment() const
	{
		return m_alignment;
	}
};