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

struct NavMeshConnection
{
	U16 connections[4];
	U32 connectionCount;
};

struct BuildNode
{
	vx::float3 position;
	U16 connections[4];
	U32 connectionCount;
};

NavMeshGraph::NavMeshGraph()
	:m_nodes(),
	m_pNavMesh(nullptr),
	m_nodeCount(0)
{
}

NavMeshGraph::~NavMeshGraph()
{

}

std::unique_ptr<NavMeshNode[]> NavMeshGraph::buildNodes(const NavMesh &navMesh, U32* finalNodeCount)
{
	auto navMeshTriangles = navMesh.getNavMeshTriangles();
	auto triangleCount = navMesh.getTriangleCount();

	auto nodeCount = triangleCount * 3;
	auto nodes = std::make_unique<BuildNode[]>(nodeCount);

	U32 vertexIndex = 0;
	for (U32 i = 0; i < triangleCount; ++i)
	{
		auto &triangle = navMeshTriangles[i];

		nodes[vertexIndex + 0].position = (triangle.m_triangle[1] + triangle.m_triangle[0]) * 0.5f;
		nodes[vertexIndex + 0].connections[0] = vertexIndex + 1;
		nodes[vertexIndex + 0].connections[1] = vertexIndex + 2;
		nodes[vertexIndex + 0].connectionCount = 2;

		nodes[vertexIndex + 1].position = (triangle.m_triangle[2] + triangle.m_triangle[1]) * 0.5f;
		nodes[vertexIndex + 1].connections[0] = vertexIndex + 0;
		nodes[vertexIndex + 1].connections[1] = vertexIndex + 2;
		nodes[vertexIndex + 1].connectionCount = 2;

		nodes[vertexIndex + 2].position = (triangle.m_triangle[0] + triangle.m_triangle[2]) * 0.5f;
		nodes[vertexIndex + 2].connections[0] = vertexIndex + 0;
		nodes[vertexIndex + 2].connections[1] = vertexIndex + 1;
		nodes[vertexIndex + 2].connectionCount = 2;

		vertexIndex += 3;
	}

	// search for duplicate vertices
	U32 current = 0;
	while (current != nodeCount)
	{
		bool foundDuplicate = false;
		U32 duplicateIndex = current;

		auto &currentNode = nodes[current];
		auto currentPosition = currentNode.position;

		for (U32 i = current + 1; i < nodeCount; ++i)
		{
			auto otherPosition = nodes[i].position;

			if (currentPosition.x == otherPosition.x &&
				currentPosition.y == otherPosition.y &&
				currentPosition.z == otherPosition.z)
			{
				foundDuplicate = true;
				duplicateIndex = i;
				break;
			}
		}

		if (foundDuplicate)
		{
			auto &duplicateNode = nodes[duplicateIndex];

			currentNode.connections[2] = duplicateNode.connections[0];
			currentNode.connections[3] = duplicateNode.connections[1];

			currentNode.connectionCount += 2;
			VX_ASSERT(currentNode.connectionCount <= 4);

			std::move(nodes.get() + duplicateIndex + 1, nodes.get() + nodeCount, nodes.get() + duplicateIndex);
			--nodeCount;
		}

		++current;
	}

	auto result = std::make_unique <NavMeshNode[]>(nodeCount);
	for (U32 i = 0; i < nodeCount; ++i)
	{
		result[i].position = nodes[i].position;
		result[i].connectionIndex = 0;
	}
	*finalNodeCount = nodeCount;

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

U32 NavMeshGraph::getNodeCount() const
{
	return m_nodeCount;
}