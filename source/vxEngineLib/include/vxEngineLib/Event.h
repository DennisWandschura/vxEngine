#pragma once

class Task;
class SmallObjAllocator;

#include <atomic>
#include <vector>

class Event
{
	friend Task;

	static std::vector<SmallObjAllocator*> s_allocators;
	static thread_local SmallObjAllocator* s_allocator;

	std::vector<Task*> m_tasks;

	void attachTask(Task* task);

public:
	Event() :m_tasks() {}
	~Event() {}

	void signal() const;

	static void* operator new(std::size_t size);

	static void operator delete(void* p, std::size_t size);

	static void setAllocator(SmallObjAllocator* allocator);
};