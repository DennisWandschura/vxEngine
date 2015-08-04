#include <vxEngineLib/TaskAllocator.h>
#include <vxEngineLib/SmallObjAllocator.h>
#include <algorithm>

u8* TaskThreadAllocator::allocate(u32 size)
{
	std::lock_guard<std::mutex> guard(m_mutex);
	auto ptr = m_allocator.allocate(size);

	if (ptr)
	{
		m_maxAlloc = std::max(m_maxAlloc, size);
		m_minAlloc = std::min(m_minAlloc, size);
	}

	return ptr;
}

bool TaskThreadAllocator::deallocate(u8* ptr, u32 size)
{
	std::lock_guard<std::mutex> guard(m_mutex);
	return m_allocator.deallocate(ptr, size);
}

thread_local TaskThreadAllocator* TaskAllocator::s_allocator{nullptr};

TaskAllocator::TaskAllocator()
{

}

TaskAllocator::~TaskAllocator()
{

}

void TaskAllocator::initialize(u32 threadCount)
{
	m_allocators.reserve(threadCount);
	for (u32 i = 0;i < threadCount; ++i)
	{
		m_allocators.push_back(nullptr);
	}
}

void TaskAllocator::registerAllocator(u32 tid, TaskThreadAllocator* alloc)
{
	m_allocators[tid] = alloc;
}

u8* TaskAllocator::allocate(u32 size)
{
	return s_allocator->allocate(size);
}

void TaskAllocator::deallocate(u8* ptr, u32 size)
{
	if (!s_allocator->deallocate(ptr, size))
	{
		for (auto &it : m_allocators)
		{
			if (it->deallocate(ptr, size))
				break;
		}
	}
}