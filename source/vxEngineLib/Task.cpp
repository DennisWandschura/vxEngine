#include <vxEngineLib/Task.h>
#include <vxEngineLib/Event.h>
#include <vxEngineLib/SmallObjAllocator.h>

std::vector<SmallObjAllocator*> Task::s_allocators{};
thread_local SmallObjAllocator* Task::s_allocator{nullptr};

void Task::attachEvent(Event* evt)
{
	evt->attachTask(this);
}

TaskReturnType Task::run()
{
	auto result = runImpl();
	if (result == TaskReturnType::Success && m_event)
	{
		m_event->signal();
	}

	return result;
}

void Task::setAllocator(SmallObjAllocator* allocator)
{
	if (s_allocator == nullptr)
	{
		s_allocator = allocator;
		s_allocators.push_back(allocator);
	}
}

void* Task::operator new(std::size_t size)
{
	return s_allocator->allocate(size);
}

void Task::operator delete(void* p, std::size_t size)
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