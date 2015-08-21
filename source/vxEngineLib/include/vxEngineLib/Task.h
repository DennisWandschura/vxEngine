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
#include <vxEngineLib/Event.h>
#include <vxEngineLib/CpuTimer.h>

class LightTask : public SmallObjectThreaded<LightTask>
{
protected:
	LightTask() {}

public:
	LightTask(const LightTask&) = delete;
	LightTask(LightTask &&rhs) = delete;

	virtual ~LightTask() {}

	LightTask& operator=(const LightTask&) = delete;
	LightTask& operator=(LightTask &&) = delete;

	virtual TaskReturnType run() = 0;

	virtual f32 getTimeMs() const = 0;
};

class Task : public LightTask
{
	Event m_event;
	std::vector<Event> m_events;
	CpuTimer m_timer;
	f32 m_timeoutTime;

	bool checkTimeout();
	void setEventStatus(EventStatus status, bool hasEvent);

protected:
	Task() :m_event(), m_events(), m_timer(), m_timeoutTime(0.0f) {}
	explicit Task(Event &&evt) :m_event(std::move(evt)), m_events() {}
	Task(Event &&evt, std::vector<Event> &&events) :m_event(std::move(evt)), m_events(std::move(events)) {}
	explicit Task(std::vector<Event> &&events) :m_event(), m_events(std::move(events)) {}

	virtual TaskReturnType runImpl() = 0;

	void setTimeoutTime(f32 timeMs) { m_timeoutTime = timeMs; }
	void setEventList(std::vector<Event>* rhs) { rhs->swap(m_events); }

public:
	Task(const Task&) = delete;
	Task(Task &&rhs) = delete;

	virtual ~Task() {}

	Task& operator=(const Task&) = delete;
	Task& operator=(Task &&) = delete;

	TaskReturnType run();

	virtual f32 getTimeMs() const = 0;
};