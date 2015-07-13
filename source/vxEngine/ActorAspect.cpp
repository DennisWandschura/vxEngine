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
#include <vxEngineLib/Locator.h>
#include <vxEngineLib/Scene.h>
#include <vxEngineLib/EventManager.h>
#include <vxEngineLib/Pool.h>
#include "ComponentActor.h"
#include "EntityAspect.h"
#include <vxEngineLib/Entity.h>
#include "EventsAI.h"
#include "AStar.h"
#include <vxEngineLib/NavNode.h>
#include <vxLib/ScopeGuard.h>
#include <vxEngineLib/EventsIngame.h>
#include <vxEngineLib/FileEvents.h>
#include "ActionManager.h"
#include "CreatedActorData.h"

ActorAspect::ActorAspect()
{

}

void ActorAspect::initialize(vx::StackAllocator* allocator, AllocationManager* allocationManager)
{
	m_squad.initialize(allocator, allocationManager);
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
		auto data = static_cast<CreatedActorData*>(evt.arg1.ptr);
		
		m_squad.addEntity(data->entity, data->componentActor,data->componentPhysics);

		delete(data);
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