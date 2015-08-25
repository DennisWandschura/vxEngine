#pragma once

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