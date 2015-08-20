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

class Task;
class SmallObjAllocator;

#include <vxEngineLib/shared_ptr.h>
#include <atomic>
#include <vector>

enum class EventStatus : s32
{
	Timeout = -2,
	Error = -1,
	Complete = 0,
	Queued = 1,
	Running = 2
};

class Event
{
	class Data;

	vx::shared_ptr<Data> m_data;

public:
	Event();
	Event(const Event &rhs);
	Event(Event &&rhs);
	~Event();

	Event& operator=(const Event &rhs);
	Event& operator=(Event &&rhs);

	void setStatus(EventStatus status);
	EventStatus getStatus() const;

	bool isValid() const;

	static Event createEvent();
	static void setAllocator(SmallObjAllocator* allocator);
};