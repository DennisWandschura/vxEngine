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

	SquadHandler::SquadHandler(U16 filterMask)
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

		auto checkIndex = [](U8 mask, U8 index) -> bool
		{
			return (mask & (1 << index)) != 0;
		};

		vx::float3 centerPosition;
		for (U8 i = 0; i < m_size; ++i)
		{
			auto actorIndex = m_actors[i];
			auto &actor = (*s_pActorPool)[actorIndex];
			auto &entity = (*s_pEntityPool)[actor.entityIndex];
			auto &physics = (*s_pPhysicsPool)[entity.physics];

			centerPosition += physics.position;
		}

		centerPosition /= F32(m_size);

		const U32 maxCount = 5u;
		auto marker = pAllocatorScratch->getMarker();
		SCOPE_EXIT
		{
			pAllocatorScratch->clear(marker);
		};

		auto pCells = reinterpret_cast<InfluenceCell*>(pAllocatorScratch->allocate(sizeof(InfluenceCell) * maxCount));

		// get cells within certain radius
		U32 count = 0;
		s_pInfluenceMap->getCells(centerPosition, 3.0f, 10.0f, maxCount, pCells, &count);

		// sort cells by current influence
		std::sort(pCells, pCells + count, [](const InfluenceCell &l, const InfluenceCell &r)
		{
			return l.m_influence < r.m_influence;
		});

		auto pNavNodeIndices = s_pInfluenceMap->getNavNodeIndices();

		U32 cellIndex = 0;
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

	void SquadHandler::updateActor(U8 index, const InfluenceCell* pCells, U32 cellIndex, const U16* pNavNodeIndices, vx::StackAllocator* pAllocatorScratch)
	{
		auto actorIndex = m_actors[index];
		auto &actor = (*s_pActorPool)[actorIndex];
		auto &entity = (*s_pEntityPool)[actor.entityIndex];
		//auto &physics = (*s_pPhysicsPool)[entity.physics];

		auto nodeIndex = 0u;
		if (pCells[cellIndex].m_count > 1)
		{
			std::uniform_int_distribution<U32> dist(0, pCells[cellIndex].m_count);

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

	void SquadHandler::updateActor(U8 index, vx::StackAllocator* pAllocatorScratch)
	{
		auto actorIndex = m_actors[index];
		auto &actor = (*s_pActorPool)[actorIndex];
		auto &entity = (*s_pEntityPool)[actor.entityIndex];
		//auto &physics = (*s_pPhysicsPool)[entity.physics];

		const U32 maxCount = 5u;
		auto marker = pAllocatorScratch->getMarker();
		auto pCells = reinterpret_cast<InfluenceCell*>(pAllocatorScratch->allocate(sizeof(InfluenceCell) * maxCount));

		SCOPE_EXIT
		{
			pAllocatorScratch->clear(marker);
		};

		// get cells within certain radius
		U32 count = 0;
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
			std::uniform_int_distribution<U32> dist(0, pCells[0].m_count);

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

	U8 SquadHandler::addActor(U16 actorIndex)
	{
		U8 result = 0;
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