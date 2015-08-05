#include <vxEngineLib/Task.h>
#include <vxEngineLib/Event.h>
#include <vxEngineLib/SmallObjAllocator.h>

TaskReturnType Task::run()
{
	for (auto &it : m_events)
	{
		if (!it->test())
		{
			return TaskReturnType::WaitingForEvents;
		}
	}

	auto result = runImpl();
	if (result == TaskReturnType::Success && m_event.ptr)
	{
		m_event->set();
	}

	return result;
}