#include "EventManager.h"
#include "Event.h"
#include "EventTypes.h"
#include "EventListener.h"

EventManager::EventManager()
	:m_events(),
	m_eventListeners()
{
}

EventManager::~EventManager()
{

}

void EventManager::initialize()
{
}

void EventManager::registerListener(EventListener* ptr, U64 priority)
{
	m_eventListeners.push_back(std::make_pair(priority, ptr));

	std::sort(m_eventListeners.begin(), m_eventListeners.end(), std::greater<>());
}

void EventManager::update()
{
	std::unique_lock<std::mutex> lck(m_evtMutex);
	m_currentReadQueue = (m_currentReadQueue + 1) % 2;
	lck.unlock();

	auto &events = m_events[m_currentReadQueue];
	for (auto &a : m_eventListeners)
	{
		for (auto &it : events)
		{
			a.second->handleEvent(it);
		}
	}
	events.clear();
}

void EventManager::addEvent(const Event &evt)
{
	std::lock_guard<std::mutex> g(m_evtMutex);
	auto currentWriteQueue = (m_currentReadQueue + 1) % 2;

	m_events[currentWriteQueue].push_back(evt);
}