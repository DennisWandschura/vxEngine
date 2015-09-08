#pragma once

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

#include <vxLib/types.h>

class Freelist
{
	u32 m_freeEntries;
	u32 m_firstFreeEntry;

public:
	Freelist() :m_freeEntries(0), m_firstFreeEntry(0){}
	~Freelist() {}

	void create(u8* begin, u32 capacity, u32 stride)
	{
		VX_ASSERT(stride >= sizeof(u32));

		for (u32 i = 0; i < capacity; ++i)
		{
			u32* freeEntry = (u32*)begin;
			*freeEntry = i + 1;

			begin += stride;
		}

		m_freeEntries = capacity;
	}

	void destroy()
	{
		m_freeEntries = 0;
		m_firstFreeEntry = 0;
	}

	u8* insertEntry(u8* begin, u32 stride)
	{
		if (m_freeEntries == 0)
			return nullptr;

		auto offsetToFreeEntry = stride * m_firstFreeEntry;
		auto ptr = begin + offsetToFreeEntry;

		auto firstFreeEntry = (u32*)ptr;
		m_firstFreeEntry = *firstFreeEntry;

		--m_freeEntries;

		return ptr;
	}

	void eraseEntry(u8* ptr, u8* begin, u32 stride, u32 capacity)
	{
		auto end = begin + stride * capacity;
		if (ptr < begin || ptr >= end)
			return;

		auto offset = ptr - begin;
		VX_ASSERT(static_cast<u32>(offset) == offset);

		auto index = static_cast<u32>(offset) / stride;

		auto freeEntry = (u32*)ptr;
		*freeEntry = m_firstFreeEntry;
		m_firstFreeEntry = index;

		++m_freeEntries;
	}
};