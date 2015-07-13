#include "AllocationManager.h"

AllocationManager::AllocationManager()
	:m_entries()
{
}

AllocationManager::~AllocationManager()
{

}

void AllocationManager::registerAllocator(const vx::Allocator* allocator, const char* text)
{
	Entry entry{};
	entry.maxSize = 0;
	entry.totalSize = allocator->getTotalSize();
	entry.minAllocSize = 0xffffffff;
	entry.maxAllocSize = 0;

	auto size = strnlen(text, 31);
	strncpy(entry.name, text, size);

	auto ptr = allocator->getMemory();

	m_entries.insert(ptr, entry);
}

void AllocationManager::updateAllocation(const u8* ptr, u32 size)
{
	auto it = m_entries.find(ptr);
	if (it != m_entries.end())
	{
		it->usedSize += size;
		it->maxSize = std::max(it->maxSize, it->usedSize);
		it->maxAllocSize = std::max(it->maxAllocSize, size);
		it->minAllocSize = std::min(it->minAllocSize, size);
	}
}

void AllocationManager::updateDeallocation(const u8* ptr, u32 size)
{
	auto it = m_entries.find(ptr);
	if (it != m_entries.end())
	{
		it->usedSize -= size;
	}
}

void AllocationManager::print() const
{
	printf("\nRegistered Allocators:\n-------------------\n");
	for (auto &it : m_entries)
	{
		printf("Name: %s, currentSize: %u, totalSize: %u, maxSize: %u, minAlloc: %u, maxAlloc: %u\n", it.name, it.usedSize, it.totalSize, it.maxSize, it.minAllocSize, it.maxAllocSize);
	}
}