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
#include "vxEngineLib/NavMeshGraph.h"
#include <vxEngineLib/NavMesh.h>
#include <vxEngineLib/NavMeshTriangle.h>
#include <vxLib/algorithm.h>
#include <vector>
#include <vxLib/Container/sorted_vector.h>

namespace NavMeshGraphCpp
{
	struct Node;
	struct Connection;

	struct BuildNode
	{
		vx::float3 position;
		vx::float3 connections[4];
		u32 connectionCount;
		u32 connectionOffset;
	};

	struct CompareFloat3
	{
		bool operator()(const vx::float3 &a, const vx::float3 &b)
		{
			if (a.x < b.x)
				return true;
			else if (a.x == b.x && a.y < b.y)
				return true;
			else if (a.x == b.x && a.y == b.y && a.z < b.z)
				return true;

			return false;
		}
	};

	struct BuildConnection
	{
		std::vector<vx::float3> connections;
	};

	void insertNode(const BuildNode &node, vx::sorted_vector<vx::float3, BuildNode, CompareFloat3>* sortedBuildNodes, vx::sorted_vector<vx::float3, BuildConnection, CompareFloat3>* connections, u32* connectionCount)
	{
		auto itNode = sortedBuildNodes->find(node.position);
		if (itNode == sortedBuildNodes->end())
		{
			sortedBuildNodes->insert(node.position, node);
		}
		else
		{
			itNode->connectionCount += node.connectionCount;
		}

		auto it = connections->find(node.position);
		if (it == connections->end())
		{
			BuildConnection connection;
			for (u32 i = 0; i < node.connectionCount; ++i)
			{
				connection.connections.push_back(node.connections[i]);
			}
			connections->insert(node.position, connection);
		}
		else
		{
			for (u32 i = 0; i < node.connectionCount; ++i)
			{
				it->connections.push_back(node.connections[i]);
			}
		}

		*connectionCount += node.connectionCount;
	}

	void insertTriangles(const NavMeshTriangle* navMeshTriangles, u32 triangleCount, vx::sorted_vector<vx::float3, BuildNode, CompareFloat3>* sortedBuildNodes, vx::sorted_vector<vx::float3, BuildConnection, CompareFloat3>* connections, u32* connectionCount)
	{
		for (u32 i = 0; i < triangleCount; ++i)
		{
			auto &triangle = navMeshTriangles[i];

			vx::float3 positions[3];
			positions[0] = (triangle.m_triangle[1] + triangle.m_triangle[0]) * 0.5f;
			positions[1] = (triangle.m_triangle[2] + triangle.m_triangle[1]) * 0.5f;
			positions[2] = (triangle.m_triangle[0] + triangle.m_triangle[2]) * 0.5f;

			BuildNode node0;
			node0.position = (triangle.m_triangle[1] + triangle.m_triangle[0]) * 0.5f;
			node0.connections[0] = positions[1];
			node0.connections[1] = positions[2];
			node0.connectionCount = 2;
			node0.connectionOffset = 0;

			BuildNode node1;
			node1.position = (triangle.m_triangle[2] + triangle.m_triangle[1]) * 0.5f;
			node1.connections[0] = positions[0];
			node1.connections[1] = positions[2];
			node1.connectionCount = 2;
			node1.connectionOffset = 0;

			BuildNode node2;
			node2.position = (triangle.m_triangle[0] + triangle.m_triangle[2]) * 0.5f;
			node2.connections[0] = positions[0];
			node2.connections[1] = positions[1];
			node2.connectionCount = 2;
			node2.connectionOffset = 0;

			insertNode(node0, sortedBuildNodes, connections, connectionCount);
			insertNode(node1, sortedBuildNodes, connections, connectionCount);
			insertNode(node2, sortedBuildNodes, connections, connectionCount);
		}
	}

	struct BuildNodesDesc
	{
		const NavMesh* navMesh;
		std::unique_ptr<NavConnection[]>* navConnections;
		std::unique_ptr<NavNode[]>* nodes;
		u32* __restrict nodeCount; 
		u32* __restrict connectionCount;
	};

	void buildNodes(const BuildNodesDesc &desc)
	{
		auto navMeshTriangles = desc.navMesh->getNavMeshTriangles();
		auto triangleCount = desc.navMesh->getTriangleCount();

		auto nodeCount = triangleCount * 3;

		vx::sorted_vector<vx::float3, BuildConnection, CompareFloat3> connections;
		connections.reserve(nodeCount);

		vx::sorted_vector<vx::float3, BuildNode, CompareFloat3> sortedBuildNodes;
		sortedBuildNodes.reserve(nodeCount);

		u32 connectionCount = 0;
		insertTriangles(navMeshTriangles, triangleCount, &sortedBuildNodes, &connections, &connectionCount);

		nodeCount = sortedBuildNodes.size();

		std::vector<NavConnection> flatConnections;
		flatConnections.reserve(connectionCount);
		u32 offset = 0;
		for (u32 i = 0; i < nodeCount; ++i)
		{
			auto &it = connections[i];
			auto position = connections.keys()[i];

			auto fromNode = sortedBuildNodes.find(position);
			fromNode->connectionOffset = offset;

			auto fromIndex = fromNode - sortedBuildNodes.begin();;

			for (auto &connection : it.connections)
			{
				auto toNode = sortedBuildNodes.find(connection);
				VX_ASSERT(toNode != sortedBuildNodes.end());

				auto toIndex = toNode - sortedBuildNodes.begin();

				NavConnection connection;
				connection.m_cost = vx::distance3(fromNode->position, toNode->position);
				connection.m_fromNode = fromIndex;
				connection.m_toNode = toIndex;

				flatConnections.push_back(connection);

				++offset;
			}
		}

		auto nodes = vx::make_unique<NavNode[]>(nodeCount);
		for (u32 i = 0; i < nodeCount; ++i)
		{
			auto &buildNode = sortedBuildNodes[i];

			auto itConnection = connections.find(buildNode.position);

			nodes[i].m_position = buildNode.position;
			nodes[i].m_connectionCount = itConnection->connections.size();
			nodes[i].m_connectionOffset = buildNode.connectionOffset;
		}

		desc.nodes->swap(nodes);
		*desc.navConnections = vx::make_unique<NavConnection[]>(connectionCount);
		vx::memcpy(desc.navConnections->get(), flatConnections.data(), connectionCount);

		*desc.nodeCount = nodeCount;
		*desc.connectionCount = connectionCount;
	}
}

NavMeshGraph::NavMeshGraph()
	:m_nodes(),
	m_connections(),
	m_pNavMesh(nullptr),
	m_nodeCount(0),
	m_connectionCount(0)
{
}

NavMeshGraph::~NavMeshGraph()
{

}

void NavMeshGraph::initialize(const NavMesh &navMesh)
{
	NavMeshGraphCpp::BuildNodesDesc desc;
	desc.connectionCount = &m_connectionCount;
	desc.navConnections = &m_connections;
	desc.navMesh = &navMesh;
	desc.nodeCount = &m_nodeCount;
	desc.nodes = &m_nodes;

	NavMeshGraphCpp::buildNodes(desc);
}

const NavNode* NavMeshGraph::getNodes() const
{
	return m_nodes.get();
}

u32 NavMeshGraph::getNodeCount() const
{
	return m_nodeCount;
}

u32 NavMeshGraph::getConnectionCount() const
{
	return m_connectionCount;
}

const NavConnection* NavMeshGraph::getConnections() const
{
	return m_connections.get();
}

u32 NavMeshGraph::getClosestNodeInex(const vx::float3 &position) const
{
	f32 distance = FLT_MAX;
	u32 index = 0;

	for (u32 i = 0; i < m_nodeCount; ++i)
	{
		auto &node = m_nodes[i];

		auto currentDistance = vx::distance3(node.m_position, position);
		if (currentDistance < distance)
		{
			distance = currentDistance;
			index = i;
		}
	}

	return index;
}