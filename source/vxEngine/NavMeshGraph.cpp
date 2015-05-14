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
#include "NavMeshGraph.h"
#include "NavMesh.h"
#include "NavMeshTriangle.h"
#include "utility.h"
#include <vector>
#include <vxLib/Container/sorted_vector.h>

struct NavMeshGraph::BuildNode
{
	vx::float3 position;
	u16 connections[4];
	u32 connectionCount;
	u16 hasDuplicate;
	u16 duplicateIndex;
	u16 newIndex;
	u16 oldIndex;
};

struct NavMeshGraph::CompareFloat3
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

void NavMeshGraph::insertTriangles(const NavMeshTriangle* navMeshTriangles, u32 triangleCount, std::vector<BuildNode>* sortedBuildNodes)
{
	u32 vertexIndex = 0;
	for (u32 i = 0; i < triangleCount; ++i)
	{
		auto &triangle = navMeshTriangles[i];

		BuildNode node;
		node.position = (triangle.m_triangle[1] + triangle.m_triangle[0]) * 0.5f;
		node.connections[0] = vertexIndex + 1;
		node.connections[1] = vertexIndex + 2;
		node.connectionCount = 2;
		node.hasDuplicate = 0;
		node.oldIndex = vertexIndex + 0;
		sortedBuildNodes->push_back(node);

		//insertOrMergeBuildNode(node, sortedBuildNodes);

		node.position = (triangle.m_triangle[2] + triangle.m_triangle[1]) * 0.5f;
		node.connections[0] = vertexIndex + 0;
		node.connections[1] = vertexIndex + 2;
		node.connectionCount = 2;
		node.hasDuplicate = 0;
		node.oldIndex = vertexIndex + 1;
		sortedBuildNodes->push_back(node);

		//insertOrMergeBuildNode(node, sortedBuildNodes);

		node.position = (triangle.m_triangle[0] + triangle.m_triangle[2]) * 0.5f;
		node.connections[0] = vertexIndex + 0;
		node.connections[1] = vertexIndex + 1;
		node.connectionCount = 2;
		node.hasDuplicate = 0;
		node.oldIndex = vertexIndex + 2;
		sortedBuildNodes->push_back(node);

		//insertOrMergeBuildNode(node, sortedBuildNodes);

		vertexIndex += 3;
	}
}

void NavMeshGraph::insertOrMergeBuildNode(const BuildNode &node, vx::sorted_vector<vx::float3, BuildNode, CompareFloat3>* sortedBuildNodes)
{
	auto it = sortedBuildNodes->find(node.position);
	if (it == sortedBuildNodes->end())
	{
		sortedBuildNodes->insert(node.position, node);
	}
	else
	{
		it->connectionCount += 2;
		it->connections[2] = node.connections[0];
		it->connections[3] = node.connections[1];
		it->hasDuplicate = 1;
		it->duplicateIndex = node.oldIndex;

		VX_ASSERT(it->connectionCount <= 4);
	}
};

void NavMeshGraph::fixConnectionIndices(u32 nodeCount, vx::sorted_vector<vx::float3, BuildNode, CompareFloat3> *sortedBuildNodes, u32* connectionCount)
{
	auto &sortedNodes = *sortedBuildNodes;

	for (u32 i = 0; i < nodeCount; ++i)
	{
		auto &buildNode = sortedNodes[i];

		*connectionCount += buildNode.connectionCount;

		/*for (u32 j = 0; j < nodeCount; ++j)
		{
			auto &otherbuildNode = (*sortedBuildNodes)[j];

			for (u32 k = 0; k < otherbuildNode.connectionCount; ++k)
			{
				if (otherbuildNode.connections[k] == buildNode.oldIndex)
				{
					otherbuildNode.connections[k] = i;
				}

				if (buildNode.hasDuplicate != 0)
				{
					if (otherbuildNode.connections[k] == buildNode.duplicateIndex)
					{
						otherbuildNode.connections[k] = i;
					}
				}
			}
		}*/
	}
}

std::unique_ptr<NavMeshNode[]> NavMeshGraph::buildNodes(const NavMesh &navMesh, u32* finalNodeCount)
{
	auto navMeshTriangles = navMesh.getNavMeshTriangles();
	auto triangleCount = navMesh.getTriangleCount();

	auto nodeCount = triangleCount * 3;

	std::vector< BuildNode> sortedBuildNodes;
	sortedBuildNodes.reserve(nodeCount);

	insertTriangles(navMeshTriangles, triangleCount, &sortedBuildNodes);

	nodeCount = sortedBuildNodes.size();
	//fixConnectionIndices(nodeCount, &sortedBuildNodes, &m_connectionCount);
	for (u32 i = 0; i < nodeCount; ++i)
	{
		auto &buildNode = sortedBuildNodes[i];

		m_connectionCount += buildNode.connectionCount;
	}

	m_connections = vx::make_unique<u16[]>(m_connectionCount);

	u32 connectionOffset = 0;
	auto result = vx::make_unique<NavMeshNode[]>(nodeCount);
	for (u32 i = 0; i < nodeCount; ++i)
	{
		auto &buildNode = sortedBuildNodes[i];
		result[i].position = buildNode.position;
		result[i].connectionIndex = connectionOffset;
		result[i].connectionCount = buildNode.connectionCount;

		for (u32 j = 0; j < buildNode.connectionCount; ++j)
		{
			m_connections[connectionOffset++] = buildNode.connections[j];
		}
	}
	*finalNodeCount = nodeCount;

	/*for (u32 j = 0; j < nodeCount; ++j)
	{
		auto &buildNode = sortedBuildNodes[j];

		for (u32 i = 0; i < m_connectionCount; ++i)
		{
			if (m_connections[i] == buildNode.oldIndex)
			{
				m_connections[i] = j;
			}
			else if (buildNode.hasDuplicate != 0 && m_connections[i] == buildNode.duplicateIndex)
			{
				m_connections[i] = j;
			}
		}
	}*/

	VX_ASSERT(connectionOffset == m_connectionCount);

	for (u32 i = 0; i < m_connectionCount; ++i)
	{
		auto connection = m_connections[i];
		if (connection >= m_nodeCount)
		{
			m_connections[i] = 0;
			//VX_ASSERT(false);
		}
	}

	return result;
}

void NavMeshGraph::initialize(const NavMesh &navMesh)
{
	m_nodes = buildNodes(navMesh, &m_nodeCount);
}

const NavMeshNode* NavMeshGraph::getNodes() const
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

const u16* NavMeshGraph::getConnections() const
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

		auto currentDistance = vx::distance(node.position, position);
		if (currentDistance < distance)
		{
			distance = currentDistance;
			index = i;
		}
	}

	return index;
}