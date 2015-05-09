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
#include "ActorAspect.h"
#include "EventTypes.h"
#include "Event.h"
#include "Locator.h"
#include "Scene.h"
#include "EventManager.h"
#include "Pool.h"
#include "ComponentActor.h"
#include "EntityAspect.h"
#include "Entity.h"
#include "ComponentPhysics.h"
#include "EventsAI.h"
#include "AStar.h"
#include "NavNode.h"
#include <vxLib/ScopeGuard.h>
#include "EventsIngame.h"

ActorAspect::ActorAspect(const PhysicsAspect &physicsAspect)
	:m_actionManager(),
	m_physicsAspect(physicsAspect)
{

}

void ActorAspect::initialize(const EntityAspect &entityAspect, EventManager &evtManager, vx::StackAllocator* pAllocator)
{
	const auto sz = 10 KBYTE;
	m_allocator = vx::StackAllocator(pAllocator->allocate(sz, 64), sz);

	const auto szSratch = 5 KBYTE;
	m_allocatorScratch = vx::StackAllocator(pAllocator->allocate(szSratch, 64), szSratch);

	m_pActorPool = &entityAspect.getActorPool();
	m_pEntityPool = &entityAspect.getEntityPool();

	evtManager.registerListener(&m_squadManager, 2);
}

void ActorAspect::shutdown()
{
	m_navGraph.shutdown(&m_allocator);
}

void ActorAspect::handleRequestPath(Component::Actor* pActor)
{
}

void ActorAspect::handleAIEvent(const Event &evt)
{
	AIEvent e = (AIEvent)evt.code;

	Component::Actor* pActor = (Component::Actor*)evt.arg1.ptr;

	switch (e)
	{
	case AIEvent::No_Orders:
		break;
	case AIEvent::Request_Path:
		handleRequestPath(pActor);
		break;
	case AIEvent::Reached_Destination:
		break;
	default:
		break;
	}
}

void ActorAspect::handleFileEvent(const Event &evt)
{
	if (evt.code == (u32)FileEvent::Scene_Loaded)
	{
		auto pCurrentScene = reinterpret_cast<const Scene*>(evt.arg1.ptr);
		auto &navmesh = pCurrentScene->getNavMesh();

		m_navGraph.initialize(navmesh, &m_allocator, &m_allocatorScratch);

		Event evt;
		evt.arg1.ptr = &m_navGraph;
		evt.type = EventType::Ingame_Event;
		evt.code = (u32)IngameEvent::Created_NavGraph;

		auto pEvtManager = Locator::getEventManager();
		pEvtManager->addEvent(evt);


		// create influence map
		f32 cellHeight = 2.1f;
		f32 cellWidthDepth = 3.0f;
		m_influenceMap.initialize(navmesh, cellWidthDepth, cellHeight);

		m_squadManager.initialize(m_pActorPool ,&m_navGraph, &m_influenceMap, nullptr, m_pEntityPool);

		Event evtInfluence;
		evtInfluence.arg1.ptr = &m_influenceMap;
		evtInfluence.type = EventType::Ingame_Event;
		evtInfluence.code = (u32)IngameEvent::Created_InfluenceMap;

		pEvtManager->addEvent(evtInfluence);
	}
}

void ActorAspect::handleIngameEvent(const Event &evt)
{
	auto ingameEvt = (IngameEvent)evt.code;

	if (ingameEvt == IngameEvent::Created_Actor)
	{
		auto pActor = (Component::Actor*)evt.arg1.ptr;

		auto index = m_pActorPool->getIndex_nocheck(pActor);
		m_squadManager.addActor(index);
	}
}

void ActorAspect::handleEvent(const Event &evt)
{
	switch (evt.type)
	{
	case EventType::AI_Event:
		handleAIEvent(evt);
		break;
	case EventType::Ingame_Event:
		handleIngameEvent(evt);
		break;
	case EventType::File_Event:
		handleFileEvent(evt);
		break;
	default:
		break;
	}
}

void ActorAspect::update(f32 dt)
{
	m_influenceMap.update(dt);

	m_actionManager.update();

	m_squadManager.update(&m_allocatorScratch);

	auto p = m_pActorPool->first();
	while (p != nullptr)
	{
		auto status = p->flags & Component::Actor::WaitingForOrders;
		if (status == Component::Actor::WaitingForOrders)
		{
			auto marker = m_allocatorScratch.getMarker();

			Action** actions = 0;
			u32 count = 0;
			p->m_stateMachine.update(&actions, &count, &m_allocatorScratch);
		
			m_actionManager.scheduleActions(actions, count);

			m_allocatorScratch.clear(marker);

			p->flags ^= Component::Actor::WaitingForOrders;
		}

		p = m_pActorPool->next_nocheck(p);
	}
}