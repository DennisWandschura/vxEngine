#include "AStar.h"
#include "NavNode.h"
#include "NavGraph.h"
#include "NavConnection.h"
#include "Heap.h"
#include "PhysicsAspect.h"
#include <vxLib/Allocator/StackAllocator.h>
#include <vxLib/ScopeGuard.h>
#include <vxLib/Container/array.h>

F32 heuristicDistance(const NavNode &fromNode, const NavNode &goalNode)
{
	auto x = goalNode.m_position.x - fromNode.m_position.x;
	auto y = goalNode.m_position.y - fromNode.m_position.y;

	return sqrtf(x * x + y * y);
}

F32 heuristicDistance2(const NavNode &fromNode, const NavNode &goalNode)
{
	auto x = goalNode.m_position.x - fromNode.m_position.x;
	auto y = goalNode.m_position.y - fromNode.m_position.y;

	return (x * x + y * y);
}

U8 pathfindAStar(const NavGraph &graph, U16 start, U16 goal, HeuristicFp heuristicFp, vx::StackAllocator* pAllocatorScratch, vx::array<vx::float3>* out, const PhysicsAspect* pPhysicsAspect)
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
		U32 nodeId;
		NavConnection connection;
		float costSoFar;
		float estaminatedTotalCost;
		U8 category{ UNVISITED };
	};

	//typedef dw::PriorityHeap<F32, NodeRecord> NodeRecordList;
	//	typedef std::map<U32, NodeRecord> NodeRecordList;

	auto graphNodes = graph.getNodes();
	auto &startNode = graphNodes[start];
	auto &goalNode = graphNodes[goal];

	// initialize the record for the start node
	NodeRecord startRecord;
	startRecord.nodeId = start;
	startRecord.connection;
	startRecord.costSoFar = 0;
	startRecord.estaminatedTotalCost = heuristicFp(startNode, goalNode);
	startRecord.category = NodeRecord::OPEN;

	auto graphConnectionCount = graph.getConnectionCount();
	// initialize the open and closed lists
	auto records = (NodeRecord*)pAllocatorScratch->allocate(sizeof(NodeRecord) * graphConnectionCount);
	pAllocatorScratch->rangeConstruct(records, records + graphConnectionCount);

	vx::HeapArray<F32, U32, std::greater<F32>> heap(graphConnectionCount, pAllocatorScratch);

	records[startRecord.nodeId] = startRecord;

	U32 current = startRecord.nodeId;
	heap.push(std::make_pair(startRecord.estaminatedTotalCost, startRecord.nodeId));

	auto graphConnections = graph.getConnections();

	U32 openSize = 1;
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
			F32 endNodeHeuristic;
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
				endNodeHeuristic = heuristicFp(graphNodes[endNode], goalNode);
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
	U8 result = 0;
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

		if (pPhysicsAspect && out->size() > 2)
		{
			auto pNewOutput = (vx::float3*)pAllocatorScratch->allocate(sizeof(vx::float3) * out->size());
			U32 newOutSize = 0;
			pNewOutput[newOutSize] = out->front();
			++newOutSize;
			//std::vector<vx::float3> newOutput;
			//newOutput.reserve(out->size());

			//newOutput.push_back(out->front());
			//U32 newOutSize = 1;


			vx::float3 hitPos;
			U32 inputIndex = 2;

			auto tmpSz = out->size() - 1;
			while (inputIndex < tmpSz)
			{
				auto p1 = (*out)[newOutSize - 1];
				auto p2 = (*out)[inputIndex];

				vx::float3 dir = vx::normalize(p2 - p1);
				//auto dir = p2 - p1;

				if (pPhysicsAspect->raycast_static(p1, dir, 10.0f, &hitPos) != 0)
				{
					//newOutput.push_back( (*out)[inputIndex - 1]);
					pNewOutput[newOutSize] = (*out)[inputIndex - 1];
					++newOutSize;
				}

				++inputIndex;
			}

			//newOutput.push_back(out->back());
			pNewOutput[newOutSize] = out->back();
			++newOutSize;

			out->clear();
			for (U32 i = 0; i < newOutSize; ++i)
			{
				out->push_back(pNewOutput[i]);
			}
			/*for (auto &it : newOutput)
			{
				out->push_back(it);
			}*/
		}

		// and return it
	}

	return result;
}