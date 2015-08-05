#include <vxEngineLib/Event.h>
#include <vxEngineLib/Task.h>
#include <vxEngineLib/SmallObjAllocator.h>

std::vector<SmallObjAllocator*> Event::s_allocators{};
thread_local SmallObjAllocator* Event::s_allocator{nullptr};

void* Event::operator new(std::size_t size)
{
	return s_allocator->allocate(size);
}

void Event::operator delete(void* p, std::size_t size)
{
	bool found = true;

	if (!s_allocator->deallocate((u8*)p, size))
	{
		found = false;
		for (auto &it : s_allocators)
		{
			if (it->deallocate((u8*)p, size))
			{
				found = true;
				break;
			}
		}
	}

	if (!found)
	{
		delete(p);
	}
}

void Event::setAllocator(SmallObjAllocator* allocator)
{
	if (s_allocator)
	{
		s_allocators.push_back(allocator);
		s_allocator = allocator;
	}
}

void Event::attachTask(Task* task)
{
	task->m_eventCount.fetch_add(1);
	m_tasks.push_back(task);
}

void Event::signal() const
{
	for (auto &it : m_tasks)
	{
		it->m_eventCount.fetch_sub(1);
	}
}