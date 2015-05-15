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

namespace ai
{
	InfluenceMap* Squad::s_influenceMap{ nullptr };
	NavMeshGraph* Squad::s_navmeshGraph{ nullptr };

	Squad::Squad()
		:m_entities()
	{

	}

	Squad::~Squad()
	{

	}

	void Squad::initialize(vx::StackAllocator* allocator)
	{
		m_scratchAllocator = vx::StackAllocator(allocator->allocate(100 KBYTE, 8), 100 KBYTE);
	}

	void Squad::addEntity(EntityActor* entity, Component::Actor* actorComponent)
	{
		Data data;
		data.entity = entity;
		data.actorComponent = actorComponent;

		m_entities.push_back(data);
	}

	void Squad::update()
	{
		for (auto &it : m_entities)
		{
			if (it.actorComponent->m_busy == 0)
			{
				auto entityPosition = it.entity->position;
				entityPosition.y = it.entity->footPositionY;

				auto endPosition = vx::float3(0, 0, -5);
				endPosition.y = entityPosition.y;

				auto startNodeIndex = s_navmeshGraph->getClosestNodeInex(entityPosition);
				auto endNodeIndex = s_navmeshGraph->getClosestNodeInex(endPosition);

				auto allocatorMarker = m_scratchAllocator.getMarker();
				SCOPE_EXIT
				{
					m_scratchAllocator.clear(allocatorMarker);
				};

				vx::array<vx::float3> outNodes = vx::array<vx::float3>(50, &m_scratchAllocator);

				astar::PathFindDescription desc;
				desc.goalIndex = endNodeIndex;
				desc.graph = s_navmeshGraph;
				desc.heuristicFp = astar::heuristicDistance;
				desc.outArray = &outNodes;
				desc.scratchAllocator = &m_scratchAllocator;
				desc.startIndex = startNodeIndex;
				desc.destinationPosition = endPosition;

				if (astar::pathfind(desc))
				{
					auto &path = it.actorComponent->m_data->path;
					path.reserve(outNodes.size());
					for (u32 i = 0; i < outNodes.size(); ++i)
					{
						path.push_back(outNodes[i]);
					}

					it.actorComponent->m_busy = 1;
					it.actorComponent->m_followingPath = 1;
				}
			}
		}
	}

	void Squad::provide(InfluenceMap* influenceMap, NavMeshGraph* graph)
	{
		s_influenceMap = influenceMap;
		s_navmeshGraph = graph;
	}
}