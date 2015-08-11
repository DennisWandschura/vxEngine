#pragma once

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

enum class TaskReturnType : unsigned int
{
	Success,
	Failure,
	Retry,
	WaitingForEvents,
	Timeout
};

namespace vx
{
	struct TaskBuffer;
	class TaskManager;
}

class SmallObjAllocator;

#include <vxLib/types.h>
#include <atomic>
#include <vector>
#include <vxEngineLib/SmallObjectThreaded.h>
#include <vxEngineLib/shared_ptr.h>
#include <vxEngineLib/Event.h>
#include <vxEngineLib/CpuTimer.h>

class Task : public SmallObjectThreaded<Task>
{
	friend vx::TaskBuffer;
	friend vx::TaskManager;

	shared_ptr<Event> m_event;
	std::vector<shared_ptr<Event>> m_events;
	CpuTimer m_timer;
	f32 m_timeoutTime;

	bool checkTimeout();
	void setEventStatus(EventStatus status);
	void setEventStatus(EventStatus status, bool hasEvent);

protected:
	virtual TaskReturnType runImpl() = 0;

	void setTimeoutTime(f32 timeMs) { m_timeoutTime = timeMs; }
	void setEventList(std::vector<shared_ptr<Event>>* rhs) { rhs->swap(m_events); }

public:
	Task() :m_event(), m_events(), m_timer(), m_timeoutTime(0.0f) {}
	explicit Task(shared_ptr<Event> &&evt) :m_event(std::move(evt)), m_events() {}
	Task(shared_ptr<Event> &&evt, std::vector<shared_ptr<Event>> &&events) :m_event(std::move(evt)), m_events(std::move(events)) {}

	Task(const Task&) = delete;
	Task(Task &&rhs) = delete;

	virtual ~Task() {}

	TaskReturnType run();

	virtual f32 getTimeMs() const = 0;
};