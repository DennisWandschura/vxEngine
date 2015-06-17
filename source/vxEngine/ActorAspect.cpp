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
#include <vxEngineLib/EventTypes.h>
#include <vxEngineLib/Event.h>
#include "Locator.h"
#include <vxEngineLib/Scene.h>
#include <vxEngineLib/EventManager.h>
#include <vxEngineLib/Pool.h>
#include "ComponentActor.h"
#include "EntityAspect.h"
#include "Entity.h"
#include "EventsAI.h"
#include "AStar.h"
#include "NavNode.h"
#include <vxLib/ScopeGuard.h>
#include "EventsIngame.h"
#include <vxEngineLib/FileEvents.h>

ActorAspect::ActorAspect(const PhysicsAspect &physicsAspect)
	:m_actionManager(),
	m_physicsAspect(physicsAspect)
{

}

void ActorAspect::initialize(const EntityAspect &entityAspect, vx::StackAllocator* pAllocator)
{
	const auto sz = 10 KBYTE;
	m_allocator = vx::StackAllocator(pAllocator->allocate(sz, 64), sz);

	const auto szSratch = 5 KBYTE;
	m_allocatorScratch = vx::StackAllocator(pAllocator->allocate(szSratch, 64), szSratch);

	m_pActorPool = &entityAspect.getActorPool();
	m_pEntityPool = &entityAspect.getEntityPool();

	m_squad.initialize(pAllocator);
}

void ActorAspect::shutdown()
{
}

void ActorAspect::createInfluenceMap(const Scene* scene)
{
	auto &navmesh = scene->getNavMesh();

	m_influenceMap.initialize(navmesh, scene->getWaypoints(), scene->getWaypointCount());
}

void ActorAspect::handleFileEvent(const vx::Event &evt)
{
	if (evt.code == (u32)vx::FileEvent::Scene_Loaded)
	{
		auto pCurrentScene = reinterpret_cast<const Scene*>(evt.arg2.ptr);
		auto &navmesh = pCurrentScene->getNavMesh();

		m_navmeshGraph.initialize(navmesh);

		vx::Event evt;
		evt.arg1.ptr = &m_navmeshGraph;
		evt.type = vx::EventType::Ingame_Event;
		evt.code = (u32)IngameEvent::Created_NavGraph;

		auto pEvtManager = Locator::getEventManager();
		pEvtManager->addEvent(evt);

		createInfluenceMap(pCurrentScene);
		vx::Event evtInfluence;
		evtInfluence.arg1.ptr = &m_influenceMap;
		evtInfluence.type = vx::EventType::Ingame_Event;
		evtInfluence.code = (u32)IngameEvent::Created_InfluenceMap;

		pEvtManager->addEvent(evtInfluence);

		ai::Squad::provide(&m_influenceMap, &m_navmeshGraph);
		m_squad.updateAfterProvide();
	}
}

void ActorAspect::handleIngameEvent(const vx::Event &evt)
{
	auto ingameEvt = (IngameEvent)evt.code;

	if (ingameEvt == IngameEvent::Created_Actor)
	{
		auto entity = static_cast<EntityActor*>(evt.arg1.ptr);
		auto actorComponent = static_cast<Component::Actor*>(evt.arg2.ptr);

		m_squad.addEntity(entity, actorComponent);
	}
}

void ActorAspect::handleEvent(const vx::Event &evt)
{
	switch (evt.type)
	{
	case vx::EventType::AI_Event:
		//handleAIEvent(evt);
		break;
	case vx::EventType::Ingame_Event:
		handleIngameEvent(evt);
		break;
	case vx::EventType::File_Event:
		handleFileEvent(evt);
		break;
	default:
		break;
	}
}

void ActorAspect::update()
{
	auto p = m_pActorPool->first();
	while (p != nullptr)
	{
		auto marker = m_allocatorScratch.getMarker();

		Action** actions = 0;
		u32 count = 0;
		p->m_stateMachine.update(&actions, &count, &m_allocatorScratch);

		m_actionManager.scheduleActions(actions, count);

		m_allocatorScratch.clear(marker);

		p = m_pActorPool->next_nocheck(p);
	}

	m_actionManager.update();
}