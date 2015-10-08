/*
The MIT License (MIT)

Copyright (c) 2015 Dennis Wandschura

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

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
	auto time = m_timer.getTimeMiliseconds();

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

	u32 count = 0;
	for (auto &it : m_events)
	{
		auto evtStatus = it.getStatus();
		if (static_cast<s32>(evtStatus) > 0)
		{
			//printf("%i %p", evtStatus, it.getAddress());
			//++count;
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