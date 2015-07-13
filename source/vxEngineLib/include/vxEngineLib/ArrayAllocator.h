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

#include <vxLib/Allocator/Allocator.h>

class ArrayAllocator : public vx::Allocator
{
	static const u32 s_maxEntrieCount = 128;

	friend class managed_ptr_base;

	struct Entry
	{
		u8* ptr;
		managed_ptr_base* managedPtr;
		u32 size;
		u16 alignment;
		u16 nextFreeEntry;

		Entry() :ptr(nullptr), managedPtr(nullptr), size(0), alignment(0), nextFreeEntry(0){}
	};

	struct DebugCheck
	{
		static const u8 s_magicCleared = 0x31;
		static const u64 s_magicSet = 0x1337b0b;

		u64 magic;
	};

	u8* m_memory;
	u8* m_head;
	u32 m_totalSize;
	u16 m_freeEntries;
	u16 m_firstFreeEntry;
	Entry m_entries[s_maxEntrieCount];
	u32 m_memoryUsed;

	void updateEntries();

	void updateEntry(u32 index, managed_ptr_base* p);

	u32 getEntrySize(u32 index) const;

	void fixHead(u8* p, u32 size);

	u8* allocate(u64 size) override;
	u8* allocate(u64 size, u8 alignment) override;
	void deallocate(u8 *ptr) override;

public:
	ArrayAllocator();

	~ArrayAllocator();

	void create(u8* memory, u32 totalSize);

	u8* release();

	managed_ptr_base allocate(u32 size, u8 alignment);

	void deallocate(managed_ptr_base* p);

	bool update();

	void update(u32 maxIterations);

	u32 getMemoryUsed() const { return m_memoryUsed; }

	u32 getTotalSize() const override;
	const u8* getMemory() const override;
};