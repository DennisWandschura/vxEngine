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
#include "SquadManager.h"
#include "Event.h"
#include "SquadHandler.h"
#include "EventTypes.h"

namespace ai
{
	u16 SquadManager::s_squadFilterMask{ 0 };

	SquadManager::SquadManager()
		:m_squadHandlers()
	{

	}

	SquadManager::~SquadManager()
	{

	}

	void SquadManager::initialize(const Pool<Component::Actor>* pActorPool, const NavGraph* pNavGraph, const InfluenceMap* pInfluenceMap, const Pool<Component::Physics>* pPhysicsPool, const Pool<EntityActor>* pEntityPool)
	{
		m_pNavGraph = pNavGraph;
		m_pInfluenceMap = pInfluenceMap;

		SquadHandler::initializeStatics(pNavGraph, pActorPool, pPhysicsPool, pEntityPool, pInfluenceMap);
	}

	void SquadManager::update(vx::StackAllocator* pAllocatorScratch)
	{
		for (auto &it : m_squadHandlers)
		{
			it.update(pAllocatorScratch);
		}
	}

	void SquadManager::handleEvent(const Event &evt)
	{
		if (evt.type == EventType::AI_Event)
		{
			handleAIEvent(evt);
		}
	}

	void SquadManager::handleAIEvent(const Event &evt)
	{
		u32 index = evt.filter;

		m_squadHandlers[index].handleAIEvent(evt);
	}

	u32 SquadManager::createSquadHandler()
	{
		auto filter = s_squadFilterMask++;

		SquadHandler newHandler(filter);

		auto result = m_squadHandlers.size();

		m_squadHandlers.push_back(newHandler);

		return result;
	}

	u32 SquadManager::addActor(u16 actorIndex)
	{
		bool found = false;
		u32 index = 0;

		auto sz = m_squadHandlers.size();
		for (u32 i = 0; i < sz; ++i)
		{
			if (m_squadHandlers[i].hasSpace())
			{
				found = true;
				index = i;
				break;
			}
		}

		if (!found)
		{
			index = createSquadHandler();
		}

		m_squadHandlers[index].addActor(actorIndex);
		return index;
	}

	const SquadHandler& SquadManager::getSquadHandler(u32 i) const
	{
		return m_squadHandlers[i];
	}

	SquadHandler& SquadManager::getSquadHandler(u32 i)
	{
		return m_squadHandlers[i];
	}
}