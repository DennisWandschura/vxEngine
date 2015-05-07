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
#include "SquadHandler.h"
#include "Event.h"
#include "EventTypes.h"
#include "EventsAI.h"
#include "ComponentActor.h"
#include "Pool.h"
#include "Entity.h"
#include "ComponentPhysics.h"
#include "InfluenceMap.h"
#include <vxLib/ScopeGuard.h>
#include <algorithm>
#include "AStar.h"
#include "NavGraph.h"
#include "Locator.h"
#include <vxLib/Allocator/StackAllocator.h>

namespace ai
{
	//const Pool<Component::Physics>* SquadHandler::s_pPhysicsPool{ nullptr };
	const Pool<Component::Actor>* SquadHandler::s_pActorPool{ nullptr };
	const Pool<EntityActor>* SquadHandler::s_pEntityPool{ nullptr };
	const InfluenceMap* SquadHandler::s_pInfluenceMap{ nullptr };
	const NavGraph* SquadHandler::s_pNavGraph{ nullptr };
	std::mt19937_64 SquadHandler::s_gen{};

	SquadHandler::SquadHandler(u16 filterMask)
		:m_filterMask(filterMask),
		m_currentTargetNodes()
	{
	}

	void SquadHandler::initializeStatics(const NavGraph* pNavGrap, const Pool<Component::Actor>* pActorPool, const Pool<Component::Physics>* pPhysicsPool, const Pool<EntityActor>* pEntityPool, const InfluenceMap* pInfluenceMap)
	{
		s_pActorPool = pActorPool;
		//s_pPhysicsPool = pPhysicsPool;
		s_pEntityPool = pEntityPool;
		s_pInfluenceMap = pInfluenceMap;
		s_pNavGraph = pNavGrap;
	}

	void SquadHandler::update(vx::StackAllocator* pAllocatorScratch)
	{
		/*if (m_updateMask == 0)
			return;

		auto checkIndex = [](u8 mask, u8 index) -> bool
		{
			return (mask & (1 << index)) != 0;
		};

		vx::float3 centerPosition;
		for (u8 i = 0; i < m_size; ++i)
		{
			auto actorIndex = m_actors[i];
			auto &actor = (*s_pActorPool)[actorIndex];
			auto &entity = (*s_pEntityPool)[actor.entityIndex];
			auto &physics = (*s_pPhysicsPool)[entity.physics];

			centerPosition += physics.position;
		}

		centerPosition /= f32(m_size);

		const u32 maxCount = 5u;
		auto marker = pAllocatorScratch->getMarker();
		SCOPE_EXIT
		{
			pAllocatorScratch->clear(marker);
		};

		auto pCells = reinterpret_cast<InfluenceCell*>(pAllocatorScratch->allocate(sizeof(InfluenceCell) * maxCount));

		// get cells within certain radius
		u32 count = 0;
		s_pInfluenceMap->getCells(centerPosition, 3.0f, 10.0f, maxCount, pCells, &count);

		// sort cells by current influence
		std::sort(pCells, pCells + count, [](const InfluenceCell &l, const InfluenceCell &r)
		{
			return l.m_influence < r.m_influence;
		});

		auto pNavNodeIndices = s_pInfluenceMap->getNavNodeIndices();

		u32 cellIndex = 0;
		if (checkIndex(m_updateMask, 0))
		{
			updateActor(0, pCells, cellIndex, pNavNodeIndices, pAllocatorScratch);
			++cellIndex;
		}

		if (checkIndex(m_updateMask, 1))
		{
			updateActor(1, pCells, cellIndex, pNavNodeIndices, pAllocatorScratch);
			++cellIndex;
		}

		if (checkIndex(m_updateMask, 2))
		{
			updateActor(2, pCells, cellIndex, pNavNodeIndices, pAllocatorScratch);
			++cellIndex;
		}

		if (checkIndex(m_updateMask, 3))
		{
			updateActor(3, pCells, cellIndex, pNavNodeIndices, pAllocatorScratch);
			++cellIndex;
		}*/

		m_updateMask = 0;
	}

	void SquadHandler::updateActor(u8 index, const InfluenceCell* pCells, u32 cellIndex, const u16* pNavNodeIndices, vx::StackAllocator* pAllocatorScratch)
	{
		auto actorIndex = m_actors[index];
		auto &actor = (*s_pActorPool)[actorIndex];
		auto &entity = (*s_pEntityPool)[actor.entityIndex];
		//auto &physics = (*s_pPhysicsPool)[entity.physics];

		auto nodeIndex = 0u;
		if (pCells[cellIndex].m_count > 1)
		{
			std::uniform_int_distribution<u32> dist(0, pCells[cellIndex].m_count);

			nodeIndex = dist(s_gen);
		}

		// get path from navgraph
		auto destNode = pNavNodeIndices[nodeIndex + pCells[cellIndex].m_offset];
		auto startNode = s_pNavGraph->getClosestNode(entity.position);

		m_currentTargetNodes[index] = destNode;
	//	printf("%u\n", destNode);

		auto result = pathfindAStar(*s_pNavGraph, startNode, destNode, heuristicDistance, pAllocatorScratch, &actor.data->path, Locator::getPhysicsAspect());
		VX_ASSERT(result != 0);

		// set data and state
		//pActor->data->path.swap(nodes);
		actor.flags |= Component::Actor::HasPath | Component::Actor::HasDestination | Component::Actor::WaitingForOrders;
	}

	void SquadHandler::updateActor(u8 index, vx::StackAllocator* pAllocatorScratch)
	{
		auto actorIndex = m_actors[index];
		auto &actor = (*s_pActorPool)[actorIndex];
		auto &entity = (*s_pEntityPool)[actor.entityIndex];
		//auto &physics = (*s_pPhysicsPool)[entity.physics];

		const u32 maxCount = 5u;
		auto marker = pAllocatorScratch->getMarker();
		auto pCells = reinterpret_cast<InfluenceCell*>(pAllocatorScratch->allocate(sizeof(InfluenceCell) * maxCount));

		SCOPE_EXIT
		{
			pAllocatorScratch->clear(marker);
		};

		// get cells within certain radius
		u32 count = 0;
		s_pInfluenceMap->getCells(entity.position, 3.0f, 10.0f, maxCount, pCells, &count);

		// sort cells by current influence
		std::sort(pCells, pCells + count, [](const InfluenceCell &l, const InfluenceCell &r)
		{
			return l.m_influence < r.m_influence;
		});

		auto pNavNodeIndices = s_pInfluenceMap->getNavNodeIndices();
		// select random nav node from cell
		auto nodeIndex = 0u;
		if (pCells[0].m_count > 1)
		{
			std::uniform_int_distribution<u32> dist(0, pCells[0].m_count);

			nodeIndex = dist(s_gen);
		}

		// get path from navgraph
		auto destNode = pNavNodeIndices[nodeIndex + pCells[0].m_offset];
		auto startNode = s_pNavGraph->getClosestNode(entity.position);

		auto result = pathfindAStar(*s_pNavGraph, startNode, destNode, heuristicDistance, pAllocatorScratch, &actor.data->path, Locator::getPhysicsAspect());
		VX_ASSERT(result != 0);

		// set data and state
		//pActor->data->path.swap(nodes);
		actor.flags |= Component::Actor::HasPath | Component::Actor::HasDestination | Component::Actor::WaitingForOrders;
	}

	void SquadHandler::handleAIEvent(const Event &evt)
	{
		VX_ASSERT(evt.filter == m_filterMask);

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

	void SquadHandler::handleRequestPath(Component::Actor* p)
	{
		auto index = s_pActorPool->getIndex_nocheck(p);
		m_updateMask |= (1 << index);
	}

	u8 SquadHandler::addActor(u16 actorIndex)
	{
		u8 result = 0;
		if (m_size < 4)
		{
			Component::Actor &actor = (*s_pActorPool)[actorIndex];
			actor.evtMask = m_filterMask;
			m_actors[m_size] = actorIndex;

			result = 1;
			++m_size;
		}

		return result;
	}
}