#include <vxEngineLib/Event.h>
#include <vxEngineLib/SmallObjectThreaded.h>

class Event::Data : public SmallObjectThreaded<Event::Data>
{
	std::atomic_int m_flag;
	std::atomic_int m_refCount;

public:
	Data() :m_flag(0), m_refCount(0) {}
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
		m_flag.store((u32)status);
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
	evt.m_data = shared_ptr<Data>(new Data());

	return evt;
}

void Event::setAllocator(SmallObjAllocator* allocator)
{
	Data::setAllocator(allocator);
}