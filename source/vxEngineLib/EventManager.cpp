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
#include <vxEngineLib/EventManager.h>
#include <vxEngineLib/Event.h>
#include <vxEngineLib/EventTypes.h>
#include <vxEngineLib/EventListener.h>

namespace vx
{
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

	void EventManager::registerListener(vx::EventListener* ptr, u64 priority, u16 filter)
	{
		Listener listener;
		listener.ptr = ptr;
		listener.mask = filter;

		m_eventListeners.push_back(std::make_pair(priority, listener));

		std::sort(m_eventListeners.begin(), m_eventListeners.end(), [](const std::pair<u64, Listener> &lhs, const std::pair<u64, Listener> &rhs)
		{
			return lhs.first > rhs.first;
		});
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
				auto m = ((u32)it.type & a.second.mask);
				if (m != 0)
				{
					a.second.ptr->handleEvent(it);
				}
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
}