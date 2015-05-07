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
#include "Pool.h"

struct Freelist
{
	static const u16 s_magic = 0x1337;

	u16 nextFreeEntry;
	u16 magic;
};

void PoolBase::initialize(u8* ptr, u16 capacity, u32 chunkSize)
{
	assert(chunkSize >= sizeof(Freelist));

	m_ptr = ptr;
	m_capacity = capacity;

	u8* pCurrent = m_ptr;
	for (auto i = 0u; i < m_capacity; ++i)
	{
		Freelist* next = reinterpret_cast<Freelist*>(pCurrent);
		next->nextFreeEntry = i + 1;
		next->magic = Freelist::s_magic;

		pCurrent += chunkSize;
	}

	m_freeEntries = m_capacity;
}

u8* PoolBase::createEntry(u16* index, u32 chunkSize)
{
	if (m_freeEntries == 0)
		return nullptr;

	auto ptr = m_ptr + m_firstFreeEntry * (chunkSize);
	validateEmptyEntry(ptr);

	*index = m_firstFreeEntry;
	m_firstFreeEntry = reinterpret_cast<Freelist*>(ptr)->nextFreeEntry;
	--m_freeEntries;

	return ptr;
}

void PoolBase::destroyEntry(u8* ptr, u16 index)
{
	auto p = reinterpret_cast<Freelist*>(ptr);
	p->nextFreeEntry = m_firstFreeEntry;
	p->magic = Freelist::s_magic;

	m_firstFreeEntry = index;
	++m_freeEntries;
}

void PoolBase::validateEmptyEntry(const u8 *ptr)
{
	auto p = reinterpret_cast<const Freelist*>(ptr);
	VX_ASSERT(p->magic == Freelist::s_magic);
}

void PoolBase::validateUsedEntry(const u8 *ptr)
{
	auto p = reinterpret_cast<const Freelist*>(ptr);
	VX_ASSERT(p->magic != Freelist::s_magic);
}

bool PoolBase::isUsed(const u8* ptr) const
{
	return (reinterpret_cast<const Freelist*>(ptr)->magic != Freelist::s_magic);
}

u8* PoolBase::release()
{
	auto p = m_ptr;

	m_ptr = nullptr;
	m_freeEntries = 0;
	m_capacity = 0;

	return p;
}

u16 PoolBase::size() const
{
	return m_capacity - m_freeEntries;
}

void* PoolBase::operator[](u16 i)
{
	return m_ptr + i;
}

const void* PoolBase::operator[](u16 i) const
{
	return m_ptr + i;
}

u8* PoolBase::first(u32 chunkSize) const
{
	auto end = m_ptr + m_capacity * chunkSize;
	u8* result = nullptr;
	u8* next = m_ptr;
	// skip unused entries
	while (next != end)
	{
		if (isUsed(next))
		{
			result = next;
			break;
		}

		next += chunkSize;
	}

	return result;
}

u8* PoolBase::next(u8* ptr, u32 chunkSize) const
{
	u8* next = ptr + chunkSize;
	auto end = m_ptr + m_capacity * chunkSize;

	u8* result = nullptr;
	// skip unused entries
	while (next != end)
	{
		if (isUsed(next))
		{
			result = next;
			break;
		}

		next += chunkSize;
	}

	return result;
}