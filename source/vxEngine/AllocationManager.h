#pragma once

namespace vx
{
	class Allocator;
}

#include <vxLib/Container/sorted_vector.h>

class AllocationManager
{
	struct Entry
	{
		u32 totalSize;
		u32 usedSize;
		u32 maxSize;
		u32 minAllocSize;
		u32 maxAllocSize;
		char name[32];
	};

	vx::sorted_vector<const u8*, Entry> m_entries;

public:
	AllocationManager();
	~AllocationManager();

	void registerAllocator(const vx::Allocator* allocator, const char* text);

	void updateAllocation(const u8* memoryStart, u32 size);
	void updateDeallocation(const u8* memoryStart, u32 size);

	void print() const;
};