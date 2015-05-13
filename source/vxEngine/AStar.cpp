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
#include "NavNode.h"
#include "NavGraph.h"
#include "NavConnection.h"
#include "Heap.h"
#include "PhysicsAspect.h"
#include <vxLib/Allocator/StackAllocator.h>
#include <vxLib/ScopeGuard.h>
#include <vxLib/Container/array.h>
#include "NavMeshGraph.h"

namespace astar
{
	f32 heuristicDistance(const vx::float3 &fromNode, const vx::float3 &goalNode)
	{
		auto x = goalNode.x - fromNode.x;
		auto y = goalNode.y - fromNode.y;
		auto z = goalNode.z - fromNode.z;

		return sqrtf(x * x + y * y + z*z);
	}

	f32 heuristicDistance2(const vx::float3 &fromNode, const vx::float3 &goalNode)
	{
		auto x = goalNode.x - fromNode.x;
		auto y = goalNode.y - fromNode.y;
		auto z = goalNode.z - fromNode.z;

		return (x * x + y * y + z * z);
	}

	u8 pathfind(const NavGraph &graph, u16 start, u16 goal, HeuristicFp heuristicFp, vx::StackAllocator* pAllocatorScratch, vx::array<vx::float3>* out)
	{
		out->clear();

		auto marker = pAllocatorScratch->getMarker();
		SCOPE_EXIT
		{
			pAllocatorScratch->clear(marker);
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

		//typedef dw::PriorityHeap<f32, NodeRecord> NodeRecordList;
		//	typedef std::map<u32, NodeRecord> NodeRecordList;

		auto graphNodes = graph.getNodes();
		auto &startNode = graphNodes[start];
		auto &goalNode = graphNodes[goal];

		// initialize the record for the start node
		NodeRecord startRecord;
		startRecord.nodeId = start;
		startRecord.connection;
		startRecord.costSoFar = 0;
		startRecord.estaminatedTotalCost = heuristicFp(startNode.m_position, goalNode.m_position);
		startRecord.category = NodeRecord::OPEN;

		auto graphConnectionCount = graph.getConnectionCount();
		// initialize the open and closed lists
		auto records = (NodeRecord*)pAllocatorScratch->allocate(sizeof(NodeRecord) * graphConnectionCount);
		pAllocatorScratch->rangeConstruct(records, records + graphConnectionCount);

		vx::HeapArray<f32, u32, std::greater<f32>> heap(graphConnectionCount, pAllocatorScratch);

		records[startRecord.nodeId] = startRecord;

		u32 current = startRecord.nodeId;
		heap.push(std::make_pair(startRecord.estaminatedTotalCost, startRecord.nodeId));

		auto graphConnections = graph.getConnections();

		u32 openSize = 1;
		while (openSize > 0)
		{
			// find smallest element in open list
			// using the estimatedTotalCost
			current = heap.top().second;

			auto &currentNode = graphNodes[current];

			// if it is the goal node, then terminate
			if (current == goal)
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
				f32 endNodeHeuristic;
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
					endNodeHeuristic = heuristicFp(graphNodes[endNode].m_position, goalNode.m_position);
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
		u8 result = 0;
		if (current == goal)
		{
			result = 1;
			// compile the list of connections in the path

			current = goal;
			// work back along the path, accumulating connections
			while (current != start)
			{
				auto connection = records[current].connection;

				auto &node = graphNodes[connection.m_toNode];
				out->push_back(node.m_position);

				current = connection.m_fromNode;
			}

			// add start node to list
			out->push_back(startNode.m_position);

			//printf("%d	%d	%u\n", start, goal, out->size());

			// reverse the path,
			//std::reverse(path.begin(), path.end());

			// and return it
		}

		return result;
	}

	bool pathfind(const PathFindDescription &desc)
	{
		desc.outArray->clear();
		VX_ASSERT(desc.outArray->capacity() != 0);

		auto marker = desc.scratchAllocator->getMarker();
		SCOPE_EXIT
		{
			desc.scratchAllocator->clear(marker);
		};

		struct Connection
		{
			f32 cost;
			u16 toNode;
			u16 fromNode;
		};

		// this structure is used to keep track of the
		// information we need for each node
		struct NodeRecord
		{
			enum class NODE_CATEGORY : u8{ OPEN, CLOSED, UNVISITED };

			u32 nodeId;
			Connection connection;
			float costSoFar;
			float estaminatedTotalCost;
			NODE_CATEGORY category{ NODE_CATEGORY::UNVISITED };
		};

		auto goalIndex = desc.goalIndex;

		auto graphNodes = desc.graph->getNodes();
		auto graphNodeCount = desc.graph->getNodeCount();

		auto graphConnectionCount = desc.graph->getConnectionCount();
		auto graphConnections = desc.graph->getConnections();

		auto &startNode = graphNodes[desc.startIndex];
		auto &goalNode = graphNodes[goalIndex];

		auto tmpConnections = (Connection*)desc.scratchAllocator->allocate(sizeof(Connection) * graphConnectionCount, 4);
		for (u32 i = 0; i < graphConnectionCount; ++i)
		{
			tmpConnections[i].cost = 0.0f;
			tmpConnections[i].toNode = graphConnections[i];
		}

		for (u32 i = 0; i < graphNodeCount; ++i)
		{
			auto &currentNode = graphNodes[i];
			auto nodeConnectionCount = currentNode.connectionCount;
			auto connectionOffset = currentNode.connectionIndex;

			for (u32 j = 0; j < nodeConnectionCount; ++j)
			{
				tmpConnections[connectionOffset + j].fromNode = i;
			}
		}

		// initialize the record for the start node
		NodeRecord startRecord;
		startRecord.nodeId = desc.startIndex;
		//startRecord.connection;
		startRecord.costSoFar = 0;
		startRecord.estaminatedTotalCost = desc.heuristicFp(startNode.position, goalNode.position);
		startRecord.category = NodeRecord::NODE_CATEGORY::OPEN;

		// initialize the open and closed lists
		auto records = (NodeRecord*)desc.scratchAllocator->allocate(sizeof(NodeRecord) * graphConnectionCount);
		desc.scratchAllocator->rangeConstruct(records, records + graphConnectionCount);

		vx::HeapArray<f32, u32, std::greater<f32>> heap(graphConnectionCount, desc.scratchAllocator);

		records[startRecord.nodeId] = startRecord;

		u32 currentIndex = startRecord.nodeId;
		heap.push(std::make_pair(startRecord.estaminatedTotalCost, startRecord.nodeId));

		u32 openSize = 1;
		while (openSize > 0)
		{
			// find smallest element in open list
			// using the estimatedTotalCost
			currentIndex = heap.top().second;

			VX_ASSERT(currentIndex <= graphNodeCount);

			auto &currentNode = graphNodes[currentIndex];

			// if it is the goal node, then terminate
			if (currentIndex == goalIndex)
				break;

			// otherwise get its outgoing connections
			auto connectionCount = currentNode.connectionCount;

			//for (auto &connection : connections)
			for (auto i = 0u; i < connectionCount; ++i)
			{
				auto &connection = tmpConnections[currentNode.connectionIndex + i];

				// get the cost estimated for the end node
				auto endNodeIndex = connection.toNode;
				auto &otherNode = graphNodes[endNodeIndex];

				if (connection.cost == 0.0f)
				{
					connection.cost = desc.heuristicFp(otherNode.position, currentNode.position);
				}

				auto endNodeCost = records[currentIndex].costSoFar + connection.cost;

				NodeRecord *pEndNodeRecord = nullptr;
				f32 endNodeHeuristic;
				// if the node is closed we may have to skip
				// or remove it from the closed list
				if (records[endNodeIndex].category == NodeRecord::NODE_CATEGORY::CLOSED)
				{
					pEndNodeRecord = &records[endNodeIndex];

					// if we didn't  find a shorter route, skip
					if (pEndNodeRecord->costSoFar <= endNodeCost)
						continue;

					// otherwise remove it from the closed list
					pEndNodeRecord->category = NodeRecord::NODE_CATEGORY::UNVISITED;

					// we can use the node's old cost values
					// to calculate its heuristic without calling
					// the possibly expensive heuristic function
					endNodeHeuristic = pEndNodeRecord->estaminatedTotalCost - pEndNodeRecord->costSoFar;
				}

				// skip if the node is open and we've not found a better route
				else if (records[endNodeIndex].category == NodeRecord::NODE_CATEGORY::OPEN)
				{
					pEndNodeRecord = &records[endNodeIndex];

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
					pEndNodeRecord = &records[endNodeIndex];
					pEndNodeRecord->nodeId = endNodeIndex;

					// we'll need to calculate the heuristic value using the function,
					// since we don't have an existing record to use
					endNodeHeuristic = desc.heuristicFp(currentNode.position, goalNode.position);
				}

				// we're here if we need to update the node
				// update cost, estimate and connection
				pEndNodeRecord->costSoFar = endNodeCost;
				pEndNodeRecord->connection = connection;
				pEndNodeRecord->estaminatedTotalCost = endNodeCost + endNodeHeuristic;

				// and add it to the open list
				if (records[endNodeIndex].category != NodeRecord::NODE_CATEGORY::OPEN)
				{
					NodeRecord *pNode = &records[endNodeIndex];
					//records[endNode] = *pEndNodeRecord;
					pNode->category = NodeRecord::NODE_CATEGORY::OPEN;
					heap.push(std::make_pair(pNode->estaminatedTotalCost, pNode->nodeId));

					++openSize;
				}
			}

			// we've finished looking at the connections for the current node,
			// so add it to the closed list
			--openSize;
			heap.pop();
			records[currentIndex].category = NodeRecord::NODE_CATEGORY::CLOSED;
		}

		// we're here if we've either found the goal, or not
		bool result = false;
		if (currentIndex == goalIndex)
		{
			result = true;
			// compile the list of connections in the path

		//	current = goalIndex;
			// work back along the path, accumulating connections
			while (currentIndex != desc.startIndex)
			{
				auto connection = records[currentIndex].connection;

				auto &node = graphNodes[connection.toNode];
				desc.outArray->push_back(node.position);

				currentIndex = connection.fromNode;
			}

			// add start node to list
			desc.outArray->push_back(startNode.position);
		}

		return result;
	}
}