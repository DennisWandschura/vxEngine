#include <vxEngineLib/Task.h>
#include <vxEngineLib/Event.h>
#include <vxEngineLib/SmallObjAllocator.h>
#include <Windows.h>

void Task::setEventStatus(EventStatus status, bool hasEvent)
{
	if (hasEvent)
	{
		m_event.setStatus(status);
	}
}

bool Task::checkTimeout()
{
	auto time = m_timer.getTimeMs();

	return (time >= m_timeoutTime);
}

TaskReturnType Task::run()
{
	auto hasEvent = m_event.isValid();

	if (m_timeoutTime != 0.0f)
	{
		if (checkTimeout())
		{
			setEventStatus(EventStatus::Timeout, hasEvent);
			return TaskReturnType::Timeout;
		}
	}

	for (auto &it : m_events)
	{
		auto evtStatus = it.getStatus();
		if (static_cast<s32>(evtStatus) > 0)
		{
			return TaskReturnType::WaitingForEvents;
		}
	}

	setEventStatus(EventStatus::Running, hasEvent);
	auto result = runImpl();

	switch (result)
	{
	case TaskReturnType::Success:
		setEventStatus(EventStatus::Complete, hasEvent);
		break;
	case TaskReturnType::Failure:
		setEventStatus(EventStatus::Error, hasEvent);
		break;
	case TaskReturnType::Retry:
		setEventStatus(EventStatus::Queued, hasEvent);
		break;
	case TaskReturnType::Timeout:
		setEventStatus(EventStatus::Error, hasEvent);
		break;
	default:
		break;
	}

	return result;
}