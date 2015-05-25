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
#include"../Entity.h"
#include "../InfluenceMap.h"
#include "../NavMeshGraph.h"
#include "../AStar.h"
#include <vxLib/Container/array.h>
#include <vxLib/ScopeGuard.h>
#include <random>
#include <vxLib/Container/sorted_vector.h>

namespace SquadCpp
{
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

	void Squad::initialize(vx::StackAllocator* allocator)
	{
		const auto memorySize = 100 KBYTE;
		m_scratchAllocator = vx::StackAllocator(allocator->allocate(memorySize, 8), memorySize);

		std::mt19937_64 gen((u64)allocator);
		std::uniform_int_distribution<u32> dist(2, 0xffffffff / 2);
		auto gen1 = dist(gen);

		std::uniform_int_distribution<u32> seedDist(1, gen1 - 1);
		auto seed = seedDist(gen);

		m_pseudoRandom = PseudoRandom(gen1, seed, 1);
	}

	bool Squad::addEntity(EntityActor* entity, Component::Actor* actorComponent)
	{
		if (m_availableCells.empty())
			return false;

		Data data;
		data.m_entity = entity;
		data.m_actorComponent = actorComponent;

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
		entityPosition.y = targetData->m_entity->footPositionY;

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

		auto influenceMapTriangles = s_influenceMap->getTriangles();

		vx::sorted_vector<f32, vx::float3> sortedCentroids;
		for (u32 i = 0; i < targetCell->triangleCount; ++i)
		{
			auto &currentTriangle = influenceMapTriangles[targetCell->triangleOffset + i];

			auto centroid = currentTriangle.getCentroid();

			auto distance = vx::distance(centroid, entityPosition);

			sortedCentroids.insert(distance, centroid);
		}

		auto keysEnd = sortedCentroids.keys() + sortedCentroids.size();
		auto it = std::lower_bound(sortedCentroids.keys(), sortedCentroids.keys() + sortedCentroids.size(), 5.0f);
		u32 triangleCount = sortedCentroids.size();
		u32 offset = 0;
		if (it != keysEnd)
		{
			triangleCount = keysEnd - it;
			offset = it - sortedCentroids.keys();
		}

		std::uniform_int_distribution<u32> dist(0, triangleCount - 1);

		auto targetTriangleIndex = dist(m_gen) + offset;

		auto endPosition = sortedCentroids[targetTriangleIndex];

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

			//printf("cell, node: %u %u\n", targetActorCellIndex, targetTriangleIndex);
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