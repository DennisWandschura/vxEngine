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
#include <vxEngineLib/MessageManager.h>
#include <vxEngineLib/Message.h>
#include <vxEngineLib/MessageTypes.h>
#include <vxEngineLib/MessageListener.h>

namespace vx
{
	MessageManager::MessageManager()
		:m_evtMutex(),
		m_events(),
		m_MessageListeners()
	{
	}

	MessageManager::~MessageManager()
	{

	}

	void MessageManager::initialize(vx::StackAllocator* allocator, u32 maxEvtCount)
	{
		auto memory = allocator->allocate(sizeof(Message) * maxEvtCount * 2, __alignof(Message));
		m_events = DoubleBuffer<Message>((Message*)memory, maxEvtCount);
	}

	void MessageManager::registerListener(MessageListener* ptr, u64 priority, u16 filter)
	{
		Listener listener;
		listener.ptr = ptr;
		listener.mask = filter;

		m_MessageListeners.push_back(std::make_pair(priority, listener));

		std::sort(m_MessageListeners.begin(), m_MessageListeners.end(), [](const std::pair<u64, Listener> &lhs, const std::pair<u64, Listener> &rhs)
		{
			return lhs.first > rhs.first;
		});
	}

	void MessageManager::update()
	{
		vx::unique_lock<vx::mutex> lck(m_evtMutex);
		m_events.swapBuffers();
		lck.unlock();

		auto evtCount = m_events.sizeBack();
		for (auto &a : m_MessageListeners)
		{
			for (u32 i = 0; i < evtCount; ++i)
			{
				auto &it = m_events.getItemFromBackBuffer(i);
				auto m = ((u32)it.type & a.second.mask);
				if (m != 0)
				{
					a.second.ptr->handleMessage(it);
				}
			}
		}
	}

	void MessageManager::addMessage(const Message &evt)
	{
		vx::lock_guard<vx::mutex> g(m_evtMutex);
		auto result = m_events.push(evt);

		VX_ASSERT(result);
	}
}