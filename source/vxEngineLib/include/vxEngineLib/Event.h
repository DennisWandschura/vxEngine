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

#include <atomic>
#include <vector>
#include <vxEngineLib/SmallObjectThreaded.h>

class Event : public SmallObjectThreaded<Event>
{
	std::atomic_int m_flag;
	std::atomic_int m_refCount;

public:
	Event() :m_flag(0), m_refCount(0) {}
	~Event() {}

	void increment()
	{
		m_refCount.fetch_add(1);
	}

	int decrement()
	{
		return m_refCount.fetch_sub(1);
	}

	void set()
	{
		m_flag.store(1);
	}

	void clear()
	{
		m_flag.store(0);
	}

	bool test() const
	{
		return (m_flag.load() != 0);
	}
};