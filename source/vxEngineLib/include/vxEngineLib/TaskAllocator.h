#pragma once

#include <vxEngineLib/SmallObjAllocator.h>
#include <mutex>

class TaskThreadAllocator
{
	SmallObjAllocator m_allocator;
	std::mutex m_mutex;
	u32 m_maxAlloc;
	u32 m_minAlloc;

public:
	TaskThreadAllocator() :m_allocator(1024), m_mutex(), m_maxAlloc(0), m_minAlloc(0xffffffff){}

	u8* allocate(u32 size);

	bool deallocate(u8* ptr, u32 size);
};

class TaskAllocator
{
	static thread_local TaskThreadAllocator* s_allocator;

	std::vector<TaskThreadAllocator*> m_allocators;

public:
	TaskAllocator();
	~TaskAllocator();

	void initialize(u32 threadCount);

	void registerAllocator(u32 tid, TaskThreadAllocator* alloc);

	u8* allocate(u32 size);

	void deallocate(u8* ptr, u32 size);
};