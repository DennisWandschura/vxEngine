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
#include <vxEngineLib/MessageTypes.h>
#include <vxEngineLib/Message.h>
#include <vxEngineLib/Locator.h>
#include <vxEngineLib/Scene.h>
#include <vxEngineLib/MessageManager.h>
#include <vxEngineLib/Pool.h>
#include "ComponentActor.h"
#include "EntityAspect.h"
#include <vxEngineLib/Entity.h>
#include "EventsAI.h"
#include "AStar.h"
#include <vxEngineLib/NavNode.h>
#include <vxLib/ScopeGuard.h>
#include <vxEngineLib/IngameMessage.h>
#include <vxEngineLib/FileMessage.h>
#include "ActionManager.h"
#include "CreatedActorData.h"

ActorAspect::ActorAspect()
{

}

void ActorAspect::initialize(vx::StackAllocator* allocator, vx::AllocationProfiler* allocationManager)
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

void ActorAspect::handleFileEvent(const vx::Message &evt)
{
	if (evt.code == (u32)vx::FileMessage::Scene_Loaded)
	{
		auto pCurrentScene = reinterpret_cast<const Scene*>(evt.arg2.ptr);
		auto &navmesh = pCurrentScene->getNavMesh();

		m_navmeshGraph.initialize(navmesh);

		vx::Message evt;
		evt.arg1.ptr = &m_navmeshGraph;
		evt.type = vx::MessageType::Ingame_Event;
		evt.code = (u32)IngameMessage::Created_NavGraph;

		auto pEvtManager = Locator::getMessageManager();
		pEvtManager->addMessage(evt);

		createInfluenceMap(pCurrentScene);

		vx::Message evtInfluence;
		evtInfluence.arg1.ptr = &m_influenceMap;
		evtInfluence.type = vx::MessageType::Ingame_Event;
		evtInfluence.code = (u32)IngameMessage::Created_InfluenceMap;

		pEvtManager->addMessage(evtInfluence);

		ai::Squad::provide(&m_influenceMap, &m_navmeshGraph);
		m_squad.updateAfterProvide();
	}
}

void ActorAspect::handleIngameMessage(const vx::Message &evt)
{
	auto ingameEvt = (IngameMessage)evt.code;

	if (ingameEvt == IngameMessage::Created_Actor)
	{
		auto data = static_cast<CreatedActorData*>(evt.arg1.ptr);
		
		m_squad.addEntity(data->entity, data->componentActor,data->componentPhysics);

		delete(data);
	}
}

void ActorAspect::handleMessage(const vx::Message &evt)
{
	switch (evt.type)
	{
	case vx::MessageType::AI_Event:
		//handleAIEvent(evt);
		break;
	case vx::MessageType::Ingame_Event:
		handleIngameMessage(evt);
		break;
	case vx::MessageType::File_Event:
		handleFileEvent(evt);
		break;
	default:
		break;
	}
}