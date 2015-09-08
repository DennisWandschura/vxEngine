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

#include <vxEngineLib/Event.h>
#include <vxEngineLib/SmallObjectThreaded.h>

class Event::Data : public SmallObjectThreaded<Event::Data>
{
	std::atomic_int m_flag;
	std::atomic_int m_refCount;

public:
	Data() :m_flag(), m_refCount() { m_flag.store(0), m_refCount.store(0); }
	~Data() {}

	void increment()
	{
		m_refCount.fetch_add(1);
	}

	int decrement()
	{
		return m_refCount.fetch_sub(1);
	}

	void setStatus(EventStatus status)
	{
		m_flag.store((s32)status);
	}

	EventStatus getStatus() const
	{
		return (EventStatus)m_flag.load();
	}
};

Event::Event()
	:m_data()
{

}

Event::Event(const Event &rhs)
	: m_data(rhs.m_data)
{

}

Event::Event(Event &&rhs)
	: m_data(std::move(rhs.m_data))
{

}

Event::~Event()
{

}

Event& Event::operator=(const Event &rhs)
{
	if (this != &rhs)
	{
		m_data = rhs.m_data;
	}

	return *this;
}

Event& Event::operator=(Event &&rhs)
{
	if (this != &rhs)
	{
		m_data = std::move(rhs.m_data);
	}

	return *this;
}

void Event::setStatus(EventStatus status)
{
	m_data->setStatus(status);
}

EventStatus Event::getStatus() const
{
	return m_data->getStatus();
}

bool Event::isValid() const
{
	return (m_data.get() != nullptr);
}

Event Event::createEvent()
{
	Event evt;
	evt.m_data = vx::shared_ptr<Data>(new Data());
	evt.setStatus(EventStatus::Queued);

	return evt;
}

void Event::setAllocator(SmallObjAllocator* allocator)
{
	Data::setAllocator(allocator);
}