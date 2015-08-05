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
	Retry
};

class SmallObjAllocator;
class Event;

#include <vxLib/types.h>
#include <atomic>
#include <vector>

class Task
{
	static std::vector<SmallObjAllocator*> s_allocators;
	static thread_local SmallObjAllocator* s_allocator;

	friend class Event;

	std::atomic_int m_eventCount;
	Event* m_event;

protected:
	virtual TaskReturnType runImpl() = 0;

public:
	Task() :m_eventCount(0), m_event(nullptr){}

	Task(const Task&) = delete;
	Task(Task &&rhs) = delete;

	virtual ~Task() {}

	void attachEvent(Event* evt);

	TaskReturnType run();

	bool isReady() const { return (m_eventCount.load() == 0); }

	virtual f32 getTimeMs() const = 0;

	static void setAllocator(SmallObjAllocator* allocator);

	static void* operator new(std::size_t size);
	static void operator delete(void* p, std::size_t size);
};