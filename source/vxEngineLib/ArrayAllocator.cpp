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

#include <vxEngineLib/ArrayAllocator.h>
#include <vxEngineLib/managed_ptr.h>
#include <algorithm>
#include <vxLib/Allocator/Allocator.h>
#include <vxLib/Allocator/AllocationProfiler.h>

ArrayAllocator::ArrayAllocator()
	:m_memory(nullptr),
	m_head(nullptr),
	m_totalSize(0),
	m_freeEntries(0),
	m_firstFreeEntry(0),
	m_memoryUsed(0),
	m_update(0),
	m_entries()
{
}

ArrayAllocator::~ArrayAllocator()
{
	if (m_memory)
	{
		if (m_memoryUsed != 0)
		{
			printf("ERROR, not all entries freed !\n");
		}

		m_memory = nullptr;
		m_head = nullptr;
		m_totalSize = 0;
		m_freeEntries = s_maxEntrieCount;
		m_firstFreeEntry = 0;
	}
}

void ArrayAllocator::create(u8* memory, u32 totalSize)
{
	VX_ASSERT(memory);

	m_memory = memory;
	m_head = memory;
	m_totalSize = totalSize;
	m_freeEntries = s_maxEntrieCount;
	m_firstFreeEntry = 0;

	memset(m_memory, DebugCheck::s_magicCleared, totalSize);

	for (u32 i = 0; i < s_maxEntrieCount; ++i)
	{
		auto &entry = m_entries[i];
		entry.nextFreeEntry = i + 1;
	}
}

u8* ArrayAllocator::release()
{
	auto p = m_memory;

	m_memory = nullptr;
	m_head = nullptr;
	m_totalSize = 0;
	m_freeEntries = s_maxEntrieCount;
	m_firstFreeEntry = 0;

	return p;
}

u8* ArrayAllocator::allocate(u64 size)
{
	return nullptr;
}

u8* ArrayAllocator::allocate(u64 size, u8 alignment)
{
	return nullptr;
}

void ArrayAllocator::deallocate(u8 *ptr)
{

}

managed_ptr_base ArrayAllocator::allocate(u32 size, u8 alignment)
{
	managed_ptr_base ptr;

	auto end = m_memory + m_totalSize;

	auto adjustment = vx::Allocator::getAdjustment(m_head, alignment);

	auto neededSize = size + sizeof(DebugCheck);

	auto alignedHead = m_head + adjustment;
	auto newHead = alignedHead + neededSize;

	if (newHead > end || m_freeEntries == 0)
		return ptr;

	auto entryIndex = m_firstFreeEntry;
	auto &entry = m_entries[entryIndex];

	auto p = alignedHead;

	entry.ptr = p;
	entry.managedPtr = &ptr;
	entry.size = neededSize;
	entry.alignment = alignment;
	m_firstFreeEntry = entry.nextFreeEntry;

	DebugCheck* debug = ((DebugCheck*)newHead) - 1;
	VX_ASSERT((u8)debug->magic == DebugCheck::s_magicCleared);
	debug->magic = DebugCheck::s_magicSet;

	m_head = newHead;

	ptr = managed_ptr_base(p, this, entryIndex);

	--m_freeEntries;
	m_memoryUsed += neededSize;

#if _VX_MEM_PROFILE
	if(s_allocationProfiler)
	{
		s_allocationProfiler->updateAllocation(m_memory, neededSize);
	}
#endif

	return ptr;
}

void ArrayAllocator::deallocate(managed_ptr_base* p)
{
	if (p->m_ptr == nullptr)
		return;

	VX_ASSERT(p->m_alloc == this);

	auto &entry = m_entries[p->m_entryIndex];
	auto size = entry.size;

	DebugCheck* debug = (DebugCheck*)(entry.ptr + size) - 1;
	VX_ASSERT(debug->magic == DebugCheck::s_magicSet);
	debug->magic = DebugCheck::s_magicCleared;

	entry.managedPtr = nullptr;

	entry.nextFreeEntry = m_firstFreeEntry;
	m_firstFreeEntry = p->m_entryIndex;
	
	fixHead(entry.ptr, size);
	memset(entry.ptr, DebugCheck::s_magicCleared, size);

	entry.ptr = nullptr;
	entry.size = 0;

	++m_freeEntries;
	m_memoryUsed -= size;

	m_update = 1;

	p->m_alloc = nullptr;
	p->m_alloc = nullptr;
	p->m_entryIndex = 0;

#if _VX_MEM_PROFILE
	if (s_allocationProfiler)
	{
		s_allocationProfiler->updateDeallocation(m_memory, size);
	}
#endif
}

bool ArrayAllocator::update()
{
	struct SortedEntry
	{
		Entry entry;
		unsigned entryIndex;
	};

	bool result = false;

	if (m_memoryUsed == 0 
		|| m_update == 0)
		return result;

	SortedEntry sortedEntries[s_maxEntrieCount];
	for (auto i = 0u; i < s_maxEntrieCount; ++i)
	{
		sortedEntries[i].entry = m_entries[i];
		sortedEntries[i].entryIndex = i;
	}

	std::sort(sortedEntries, sortedEntries + s_maxEntrieCount, [](const SortedEntry &lhs, const SortedEntry &rhs) -> bool
	{
		return (lhs.entry.ptr > rhs.entry.ptr);
	});

	auto last = &sortedEntries[0];

	SortedEntry* first = sortedEntries;
	while (true)
	{
		auto next = first + 1;
		if (next->entry.ptr == nullptr)
			break;

		first = next;
	}

	if (first == last)
		return result;

	auto current = first;
	while (true)
	{
		if (current == last)
			break;

		auto next = current - 1;

		auto currentSize = current->entry.size;
		auto currentPtrEnd = current->entry.ptr + currentSize;

		auto nextPtr = next->entry.ptr;
		auto nextSize = next->entry.size;
		DebugCheck* nextDebug = (DebugCheck*)(nextPtr + nextSize) - 1;

		auto nextAlignment = next->entry.alignment;

		auto adjustment = vx::Allocator::getAdjustment(currentPtrEnd, nextAlignment);
		auto currentPtrEndAligned = currentPtrEnd + adjustment;

		auto distance = nextPtr - currentPtrEndAligned;

		// distance to next entry is large enough to fit data
		if (distance >= nextSize)
		{
			VX_ASSERT(nextDebug->magic == DebugCheck::s_magicSet);

			auto oldPtr = nextPtr;
			auto nextManagedPtr = next->entry.managedPtr;

			auto newPtr = currentPtrEndAligned;
			DebugCheck* newDebug = (DebugCheck*)(newPtr + nextSize) - 1;
			memcpy(newPtr, oldPtr, nextSize);
			newDebug->magic = DebugCheck::s_magicSet;

			next->entry.ptr = newPtr;
			nextManagedPtr->m_ptr = newPtr;

			memset(oldPtr, 0, nextSize);

			//printf("moved %p to %p\n", oldPtr, newPtr);

			m_entries[next->entryIndex] = next->entry;

			fixHead(oldPtr, nextSize);

			result = true;
		}

		current = next;
	}

	m_update = 0;

	return result;
}

void ArrayAllocator::update(u32 maxIterations)
{
	while (maxIterations != 0)
	{
		if (!update())
		{
			break;
		}

		--maxIterations;
	}
}

void ArrayAllocator::updateEntries()
{
	for (u32 i = 0; i < s_maxEntrieCount; ++i)
	{
		auto &entry = m_entries[i];

		if (entry.managedPtr != nullptr)
		{
			entry.managedPtr->setPtr(entry.ptr);
		}
	}
}

void ArrayAllocator::updateEntry(u32 index, managed_ptr_base* p)
{
	auto &entry = m_entries[index];
	entry.managedPtr = p;
}

unsigned ArrayAllocator::getEntrySize(u32 index) const
{
	auto &entry = m_entries[index];
	return entry.size;
}

void ArrayAllocator::fixHead(u8* p, u32 size)
{
	auto last = p + size;
	if (last == m_head)
	{
		m_head -= size;
	}
}

u32 ArrayAllocator::getTotalSize() const
{
	return m_totalSize;
}

const u8* ArrayAllocator::getMemory() const
{
	return m_memory;
}