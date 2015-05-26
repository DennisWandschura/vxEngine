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
#include "EventManager.h"
#include <vxEngineLib/Event.h>
#include <vxEngineLib/EventTypes.h>
#include <vxEngineLib/EventListener.h>

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

void EventManager::registerListener(vx::EventListener* ptr, u64 priority)
{
	m_eventListeners.push_back(std::make_pair(priority, ptr));

	std::sort(m_eventListeners.begin(), m_eventListeners.end(), std::greater<std::pair<u64, vx::EventListener*>>());
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

void EventManager::addEvent(const vx::Event &evt)
{
	std::lock_guard<std::mutex> g(m_evtMutex);
	auto currentWriteQueue = (m_currentReadQueue + 1) % 2;

	m_events[currentWriteQueue].push_back(evt);
}