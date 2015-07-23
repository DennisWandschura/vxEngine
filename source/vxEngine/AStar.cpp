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
#include "AStar.h"
#include <vxEngineLib/NavNode.h>
#include <vxEngineLib/NavGraph.h>
#include <vxEngineLib/NavConnection.h>
#include "Heap.h"
#include "PhysicsAspect.h"
#include <vxLib/Allocator/StackAllocator.h>
#include <vxLib/ScopeGuard.h>
#include <vxLib/Container/array.h>
#include <vxEngineLib/NavMeshGraph.h>
#include <vxEngineLib/Timer.h>

f32 AStar::heuristicDistance(const vx::float3 &fromNode, const vx::float3 &goalNode)
{
	auto L = goalNode - fromNode;
	return sqrtf(vx::dot3(L, L));
}

f32 AStar::heuristicDistance2(const vx::float3 &fromNode, const vx::float3 &goalNode)
{
	auto L = goalNode - fromNode;
	return vx::dot3(L, L);
}

bool AStar::pathfind(const PathFindDescription &desc)
{
	Timer timer;

	desc.outArray->clear();
	VX_ASSERT(desc.outArray->capacity() != 0);

	auto marker = desc.scratchAllocator->getMarker();
	SCOPE_EXIT
	{
		desc.scratchAllocator->clear(marker);
	};

	// this structure is used to keep track of the
	// information we need for each node
	struct NodeRecord
	{
		enum NODE_CATEGORY{ OPEN, CLOSED, UNVISITED };
		u32 nodeId;
		NavConnection connection;
		float costSoFar;
		float estaminatedTotalCost;
		u8 category{ UNVISITED };
	};

	auto goalIndex = desc.goalIndex;

	auto graphNodes = desc.graph->getNodes();

	auto graphConnectionCount = desc.graph->getConnectionCount();
	auto graphConnections = desc.graph->getConnections();

	auto &startNode = graphNodes[desc.startIndex];
	auto &goalNode = graphNodes[goalIndex];

	// initialize the record for the start node
	NodeRecord startRecord;
	startRecord.nodeId = desc.startIndex;
	startRecord.connection;
	startRecord.costSoFar = 0;
	startRecord.estaminatedTotalCost = desc.heuristicFp(startNode.m_position, goalNode.m_position);
	startRecord.category = NodeRecord::OPEN;

	// initialize the open and closed lists
	auto records = (NodeRecord*)desc.scratchAllocator->allocate(sizeof(NodeRecord) * graphConnectionCount);
	desc.scratchAllocator->rangeConstruct(records, records + graphConnectionCount);

	vx::HeapArray<f32, u32, std::greater<f32>> heap(graphConnectionCount, desc.scratchAllocator);

	records[startRecord.nodeId] = startRecord;

	u32 current = startRecord.nodeId;
	heap.push(std::make_pair(startRecord.estaminatedTotalCost, startRecord.nodeId));

	u32 openSize = 1;
	while (openSize > 0)
	{
		// find smallest element in open list
		// using the estimatedTotalCost
		current = heap.top().second;

		auto &currentNode = graphNodes[current];

		// if it is the goal node, then terminate
		if (current == goalIndex)
			break;

		// otherwise get its outgoing connections
		auto connectionCount = currentNode.m_connectionCount;

		//for (auto &connection : connections)
		for (auto i = 0u; i < connectionCount; ++i)
		{
			auto &connection = graphConnections[currentNode.m_connectionOffset + i];

			// get the cost estimated for the end node
			auto endNode = connection.m_toNode;
			auto endNodeCost = records[current].costSoFar + connection.m_cost;

			NodeRecord *pEndNodeRecord = nullptr;
			f32 endNodeHeuristic = 0.0f;
			// if the node is closed we may have to skip
			// or remove it from the closed list
			if (records[endNode].category == NodeRecord::CLOSED)
			{
				pEndNodeRecord = &records[endNode];

				// if we didn't  find a shorter route, skip
				if (pEndNodeRecord->costSoFar <= endNodeCost)
					continue;

				// otherwise remove it from the closed list
				pEndNodeRecord->category = NodeRecord::UNVISITED;

				// we can use the node's old cost values
				// to calculate its heuristic without calling
				// the possibly expensive heuristic function
				endNodeHeuristic = pEndNodeRecord->estaminatedTotalCost - pEndNodeRecord->costSoFar;
			}

			// skip if the node is open and we've not found a better route
			else if (records[endNode].category == NodeRecord::OPEN)
			{
				pEndNodeRecord = &records[endNode];

				// if our route is no better, then skip
				if (pEndNodeRecord->costSoFar <= endNodeCost)
					continue;

				// we can use the node's old cost values
				// to calculate its heuristic without calling
				// the possibly expensive heuristic function
				endNodeHeuristic = pEndNodeRecord->estaminatedTotalCost - pEndNodeRecord->costSoFar;
			}

			// otherwise we know we've got an unvisited
			// node, so make a record for it
			else
			{
				pEndNodeRecord = &records[endNode];
				pEndNodeRecord->nodeId = endNode;

				// we'll need to calculate the heuristic value using the function,
				// since we don't have an existing record to use
				endNodeHeuristic = desc.heuristicFp(graphNodes[endNode].m_position, goalNode.m_position);
			}

			// we're here if we need to update the node
			// update cost, estimate and connection
			pEndNodeRecord->costSoFar = endNodeCost;
			pEndNodeRecord->connection = connection;
			pEndNodeRecord->estaminatedTotalCost = endNodeCost + endNodeHeuristic;

			// and add it to the open list
			if (records[endNode].category != NodeRecord::OPEN)
			{
				auto *pNode = &records[endNode];
				//records[endNode] = *pEndNodeRecord;
				pNode->category = NodeRecord::OPEN;
				heap.push(std::make_pair(pNode->estaminatedTotalCost, pNode->nodeId));

				++openSize;
			}
		}

		// we've finished looking at the connections for the current node,
		// so add it to the closed list
		--openSize;
		heap.pop();
		records[current].category = NodeRecord::CLOSED;
	}

	// we're here if we've either found the goal, or not
	bool result = false;
	if (current == goalIndex)
	{
		result = true;
		// compile the list of connections in the path

		desc.outArray->push_back(desc.destinationPosition);

		//	current = goalIndex;
		// work back along the path, accumulating connections
		while (current != desc.startIndex)
		{
			auto connection = records[current].connection;

			auto &node = graphNodes[connection.m_toNode];
			desc.outArray->push_back(node.m_position);

			current = connection.m_fromNode;
		}

		// add start node to list
		desc.outArray->push_back(startNode.m_position);
	}

	auto time = timer.getTimeInMs();
	//printf("Astar time: %f ms\n", time);

	return result;
}