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
#include "vxEngineLib/NavGraph.h"
#include <vxEngineLib/NavMesh.h>
#include <vxEngineLib/AABB.h>
#include "vxEngineLib/NavConnection.h"
#include "vxEngineLib/NavNode.h"
#include <vector>
#include <vxLib/Allocator/StackAllocator.h>
#include <vxLib/algorithm.h>
#include <vxLib/ScopeGuard.h>

namespace 
{
	struct NavGraphTriangle
	{
		vx::float3 v[3];
		vx::float3 c;
		u16 sharedEdges[3];
		u8 sharedEdgesCount{ 0 };

		bool sharesEdge(const NavGraphTriangle &other)
		{
			if (this == &other)
				return false;

			u8 count = 0;

			for (u32 i = 0; i < 3; ++i)
			{
				auto &vertex = v[i];
				for (u32 j = 0; j < 3; ++j)
				{
					auto &otherVertex = other.v[j];

					auto result = (vertex.x == otherVertex.x) & (vertex.y == otherVertex.y) & (vertex.z == otherVertex.z);

					count += result;
				}
			}

			return (count > 1);
		}
	};

	struct BuildCell
	{
		std::vector<u32> indices;
	};
}

NavGraph::NavGraph()
#if _VX_EDITOR
	:m_connections(),
	m_nodes(),
#else
	:m_connections(nullptr),
	m_nodes(nullptr),
#endif
	m_connectionCount(0),
	m_nodeCount(0)
{
}

NavGraph::~NavGraph()
{

}

void NavGraph::initialize(const NavMesh &navMesh, vx::StackAllocator* pAllocator, vx::StackAllocator* pAllocatorScratch)
{
	auto vertices = navMesh.getVertices();
	auto triangleIndices = navMesh.getTriangleIndices();

	auto triangleCount = navMesh.getTriangleCount();

	auto scratchMarker = pAllocatorScratch->getMarker();

	SCOPE_EXIT
	{
		pAllocatorScratch->clear(scratchMarker);
	};

	auto triangles = (NavGraphTriangle*)pAllocatorScratch->allocate(sizeof(NavGraphTriangle) * triangleCount, __alignof(NavGraphTriangle));
	VX_ASSERT(triangles);

	// bounds of centroids
	AABB bounds;
	// create triangles

	u32 index=0;
	for (u32 i = 0; i < triangleCount; ++i)
	{
		NavGraphTriangle t;
		t.v[0] = vertices[triangleIndices[index++]];
		t.v[1] = vertices[triangleIndices[index++]];
		t.v[2] = vertices[triangleIndices[index++]];
		t.c = (t.v[0] + t.v[1] + t.v[2]) / 3.0f;

		bounds = AABB::merge(bounds, t.c);

		triangles[i] = t;
	}

	// sort triangles by position
	std::sort(triangles, triangles + triangleCount, [](const NavGraphTriangle &lhs, const NavGraphTriangle &rhs)
	{
		return (
			(lhs.c.x < rhs.c.x) ||
			(lhs.c.x == rhs.c.x && lhs.c.y < rhs.c.y) ||
			(lhs.c.x == rhs.c.x && lhs.c.y == rhs.c.y && lhs.c.z < rhs.c.z)
			);
	});

	u32 connectionCount = 0;
	// find triangles that share edges
	for (u32 j = 0; j < triangleCount; ++j)
	{
		auto &it = triangles[j];

		u8 sharedCount = 0;
		for (u32 i = 0; i < triangleCount; ++i)
		{
			if (it.sharesEdge(triangles[i]))
			{
				VX_ASSERT(it.sharedEdgesCount < 3);

				it.sharedEdges[it.sharedEdgesCount++] = i;
				++sharedCount;
			}
		}

		connectionCount += sharedCount;
	}

	auto connections = (NavConnection*)pAllocatorScratch->allocate(sizeof(NavConnection) * connectionCount, __alignof(NavConnection));
	u32 connectionOffset = 0;

	m_nodeCount = triangleCount;
	m_nodes = (NavNode*)pAllocator->allocate(sizeof(NavNode) * m_nodeCount, __alignof(NavNode));

	for (u32 i = 0; i < triangleCount; ++i)
	{
		auto &triangle = triangles[i];

		NavNode node;
		node.m_connectionCount = triangle.sharedEdgesCount;
		node.m_connectionOffset = connectionOffset;
		node.m_position = triangle.c;

		m_nodes[i] = node;

		for (u32 j = 0; j < triangle.sharedEdgesCount; ++j)
		{
			auto it = triangle.sharedEdges[j];

			NavConnection connection;
			connection.m_fromNode = i;
			connection.m_toNode = it;

			auto &toTriangle = triangles[it];

			connection.m_cost = vx::distance3(triangle.c, toTriangle.c);

			connections[connectionOffset++] = connection;
		}
	}

	VX_ASSERT(connectionCount == connectionOffset);

	m_connections = (NavConnection*)pAllocator->allocate(sizeof(NavConnection) * connectionCount, __alignof(NavConnection));
	vx::memcpy(m_connections, connections, connectionCount);

	m_connectionCount = connectionCount;
}

void NavGraph::shutdown(vx::StackAllocator* pAllocator)
{
	pAllocator->rangeDestroy(m_connections, m_connections + m_connectionCount);
	pAllocator->rangeDestroy(m_nodes, m_nodes + m_nodeCount);
}

u32 NavGraph::getClosestNode(const vx::float3 &position) const
{
	f32 distance = FLT_MAX;
	u32 nodeIndex = -1;
	for (u32 i = 0; i < m_nodeCount; ++i)
	{
		auto &node = m_nodes[i];

		auto currentDistance = vx::distance3(position, node.m_position);

		if (currentDistance < distance)
		{
			distance = currentDistance;
			nodeIndex = i;
		}
	}

	return nodeIndex;
}

u32 NavGraph::getFarestNode(const vx::float3 &position) const
{
	f32 distance = -FLT_MAX;
	u32 nodeIndex = -1;
	for (u32 i = 0; i < m_nodeCount; ++i)
	{
		auto &node = m_nodes[i];

		auto currentDistance = vx::distance3(position, node.m_position);

		if (currentDistance > distance)
		{
			distance = currentDistance;
			nodeIndex = i;
		}
	}

	return nodeIndex;
}

const NavNode& NavGraph::getNode(u32 i) const
{
	return m_nodes[i];
}