#include <vxEngineLib/Task.h>
#include <vxEngineLib/Event.h>
#include <vxEngineLib/SmallObjAllocator.h>
#include <Windows.h>

bool Task::checkTimeout()
{
	auto time = m_timer.getTimeMs();

	return (time >= m_timeoutTime);
}

TaskReturnType Task::run()
{
	if (m_timeoutTime != 0.0f)
	{
		if (checkTimeout())
		{
			return TaskReturnType::Timeout;
		}
	}

	for (auto &it : m_events)
	{
		if (!it->test())
		{
			return TaskReturnType::WaitingForEvents;
		}
	}

	auto result = runImpl();
	if (result == TaskReturnType::Success && m_event.get() != nullptr)
	{
		m_event->set();
	}

	return result;
}