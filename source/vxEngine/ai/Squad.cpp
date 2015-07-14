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

#include "Squad.h"
#include "../ComponentActor.h"
#include <vxEngineLib/Entity.h>
#include <vxEngineLib/InfluenceMap.h>
#include <vxEngineLib/NavMeshGraph.h>
#include "../AStar.h"
#include <vxLib/Container/array.h>
#include <vxLib/ScopeGuard.h>
#include <random>
#include <vxEngineLib/Waypoint.h>
#include <vxLib/Container/sorted_vector.h>
#include <vxEngineLib/Locator.h>
#include "../PhysicsAspect.h"
#include "../ComponentPhysics.h"
#include <vxLib/Allocator/AllocationProfiler.h>

namespace SquadCpp
{
	void smoothPath(f32 collisionRadius, const vx::array<vx::float3> &oldPath, std::vector<vx::float3>* path)
	{
		auto physicsAspect = Locator::getPhysicsAspect();

		auto size = oldPath.size();
		if (size <= 2)
			return;

		const vx::float3 upDir = { 0, 1, 0 };
		const __m128 vUpDir = { 0, 1, 0, 0};

		vx::float3 hitPosition;
		f32 hitDistance;

		path->reserve(size);

		auto srcPos = oldPath[0];
		path->push_back(srcPos);

		auto p0 = vx::loadFloat3(srcPos);

		const __m128 vRadius = { collisionRadius,collisionRadius, collisionRadius, 0};

		for (u32 i = 2; i < size; ++i)
		{
			auto &dstPos = oldPath[i];
			auto p1 = vx::loadFloat3(dstPos);

			auto D = _mm_sub_ps(p1, p0);
			auto vDistance = vx::length3(D);
			vx::float4a direction = _mm_div_ps(D, vDistance);
			auto tangent = vx::cross3(vUpDir, direction);

			auto pos0 = _mm_fmadd_ps(vRadius, tangent, p0);
			auto pos1 = _mm_fmadd_ps(vRadius, vx::negate(tangent), p0);

			f32 distance = 0.0f;
			_mm_store_ss(&distance, vDistance);

			/*auto D = dstPos - srcPos;
			auto distance = vx::length3(D);
			auto direction = D / distance;
			auto tangent = vx::cross(upDir, direction);

			auto pos0 = srcPos + tangent * collisionRadius;
			auto pos1 = srcPos - tangent * collisionRadius;*/

			auto sid0 = physicsAspect->raycast_static(pos0, direction, distance, &hitPosition, &hitDistance);
			auto sid1 = physicsAspect->raycast_static(pos1, direction, distance, &hitPosition, &hitDistance);

			if (sid0.value != 0 ||
				sid1.value != 0)
			{
				path->push_back(oldPath[i - 1]);
				srcPos = oldPath[i];
				path->push_back(srcPos);

				p0 = vx::loadFloat3(srcPos);
			}
		}

		//newPath.push_back((*path)[src]);
		path->push_back(oldPath.back());
	}
}

namespace ai
{
	InfluenceMap* Squad::s_influenceMap{ nullptr };
	NavMeshGraph* Squad::s_navmeshGraph{ nullptr };

	Squad::Squad()
		:m_entities(),
		m_pseudoRandom(),
		m_avgCoverageArea(30.0f)
	{

	}

	Squad::~Squad()
	{

	}

	void Squad::initialize(vx::StackAllocator* allocator, vx::AllocationProfiler* allocationManager)
	{
		const auto memorySize = 100 KBYTE;
		m_scratchAllocator = vx::StackAllocator(allocator->allocate(memorySize, 8), memorySize);
#if _VX_MEM_PROFILE
		allocationManager->registerAllocator(&m_scratchAllocator, "SquadAlloc");
#endif

		std::mt19937_64 gen((u64)allocator);
		std::uniform_int_distribution<u32> dist(2, 0xffffffff / 2);
		auto gen1 = dist(gen);

		std::uniform_int_distribution<u32> seedDist(1, gen1 - 1);
		auto seed = seedDist(gen);

		m_pseudoRandom = PseudoRandom(gen1, seed, 1);
		m_pseudoRandom.setMaxValue(1024);
	}

	bool Squad::addEntity(Entity* entity, Component::Actor* actorComponent, Component::Physics* componentPhysics)
	{
		if (m_availableCells.empty())
			return false;

		Data data;
		data.m_entity = entity;
		data.m_actorComponent = actorComponent;
		data.m_componentPhysics = componentPhysics;

		auto influenceCells = s_influenceMap->getCells();

		u8 cellCount = 0;

		if (!m_availableCells.empty())
		{
			auto it = m_availableCells.begin();

			auto firstCellIndex = *it;
			data.m_cells[0] = firstCellIndex;
			++cellCount;

			it = m_availableCells.erase(it);

			while (it != m_availableCells.end())
			{
				if (s_influenceMap->sharesEdge(influenceCells[firstCellIndex], influenceCells[*it]))
				{
					data.m_cells[1] = *it;
					++cellCount;

					//m_waypoints;

					m_availableCells.erase(it);
					break;
				}
			}
		}

		data.m_cellCount = cellCount;
		actorComponent->m_data->squad = this;
		actorComponent->m_data->targetCell = -1;

		m_entities.push_back(data);

		return true;
	}

	void Squad::createPath(Component::Actor* componentActor)
	{
		Data* targetData = nullptr;
		for (auto &it : m_entities)
		{
			if (it.m_actorComponent == componentActor)
			{
				targetData = &it;
				break;
			}
		}

		if (targetData == nullptr)
			return;

		auto entityPosition = targetData->m_entity->position;
		entityPosition.y = targetData->m_componentPhysics->footPositionY;

		auto influenceCells = s_influenceMap->getCells();
		auto influenceCellBounds = s_influenceMap->getBounds();

		auto currentActorCellIndex = targetData->m_actorComponent->m_data->targetCell;
		auto actorCellCount = targetData->m_cellCount;

		if (currentActorCellIndex == -1)
		{
			for (u32 i = 0; i < actorCellCount; ++i)
			{
				auto cellIndex = targetData->m_cells[i];
				if (influenceCellBounds[cellIndex].contains(entityPosition))
				{
					currentActorCellIndex = i;
					
					break;
				}
			}
		}
		
		auto cellIndex = targetData->m_cells[currentActorCellIndex];
		const InfluenceCell* currentCell = &influenceCells[cellIndex];
		auto targetActorCellIndex = currentActorCellIndex;

		const InfluenceCell* targetCell = currentCell;
		if (actorCellCount == 2)
		{
			auto otherActorCellIndex = (currentActorCellIndex + 1) % 2;
			auto cellIndex = targetData->m_cells[otherActorCellIndex];

			targetCell = &influenceCells[cellIndex];
			targetActorCellIndex = otherActorCellIndex;
		}

		if (targetCell == currentCell)
			printf("Squad::createPath: Something went wrong\n");

		auto waypoints = s_influenceMap->getWaypoints();

		std::vector<vx::float4> sortedWaypoints;
		for (u32 i = 0; i < targetCell->waypointCount; ++i)
		{
			auto &waypoint = waypoints[targetCell->waypointOffset + i];
			auto distance = vx::distance3(waypoint.position, entityPosition);

			vx::float4 p;
			p.x = waypoint.position.x;
			p.y = waypoint.position.y;
			p.z = waypoint.position.z;
			p.w = distance;

			sortedWaypoints.push_back(p);
		}

		std::sort(sortedWaypoints.begin(), sortedWaypoints.end(), [](const vx::float4 &l, const vx::float4 &r)
		{
			return l.w > r.w;
		});

		auto index = m_pseudoRandom.getValue() % targetCell->waypointCount;

		if (sortedWaypoints[index].w <= 5.0f)
			index = (index + 1) % targetCell->waypointCount;

		vx::float3 endPosition;
		endPosition.x = sortedWaypoints[index].x;
		endPosition.y = sortedWaypoints[index].y;
		endPosition.z = sortedWaypoints[index].z;

		auto startNodeIndex = s_navmeshGraph->getClosestNodeInex(entityPosition);
		auto endNodeIndex = s_navmeshGraph->getClosestNodeInex(endPosition);

		auto allocatorMarker = m_scratchAllocator.getMarker();
		SCOPE_EXIT
		{
			m_scratchAllocator.clear(allocatorMarker);
		};

		vx::array<vx::float3> outNodes = vx::array<vx::float3>(50, &m_scratchAllocator);

		AStar::PathFindDescription desc;
		desc.goalIndex = endNodeIndex;
		desc.graph = s_navmeshGraph;
		desc.heuristicFp = AStar::heuristicDistance;
		desc.outArray = &outNodes;
		desc.scratchAllocator = &m_scratchAllocator;
		desc.startIndex = startNodeIndex;
		desc.destinationPosition = endPosition;

		if (AStar::pathfind(desc))
		{
			auto &path = targetData->m_actorComponent->m_data->path;
			path.reserve(outNodes.size());
			for (u32 i = 0; i < outNodes.size(); ++i)
			{
				path.push_back(outNodes[i]);
			}

			//SquadCpp::smoothPath(0.3f, outNodes, &path);

		//	printf("destination: %f %f %f\n", endPosition.x, endPosition.y, endPosition.z);
			targetData->m_actorComponent->m_followingPath = 1;
			targetData->m_actorComponent->m_data->targetCell = targetActorCellIndex;
		}
	}

	void Squad::provide(InfluenceMap* influenceMap, NavMeshGraph* graph)
	{
		s_influenceMap = influenceMap;
		s_navmeshGraph = graph;
	}

	void Squad::updateAfterProvide()
	{
		auto cellCount = s_influenceMap->getCellCount();

		for (u32 i = 0; i < cellCount; ++i)
			m_availableCells.push_back(i);
	}
}