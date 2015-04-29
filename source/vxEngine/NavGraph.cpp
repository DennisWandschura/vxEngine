#include "NavGraph.h"
#include "NavMesh.h"
#include "NavConnection.h"
#include "NavNode.h"
#include <vector>
#include "utility.h"
#include <vxLib/Allocator/StackAllocator.h>
#include <algorithm>
#include "AABB.h"
#include <vxLib/ScopeGuard.h>

namespace 
{
	struct NavGraphTriangle
	{
		vx::float3 v[3];
		vx::float3 c;
		U16 sharedEdges[3];
		U8 sharedEdgesCount{ 0 };

		bool sharesEdge(const NavGraphTriangle &other)
		{
			if (this == &other)
				return false;

			U8 count = 0;

			for (U32 i = 0; i < 3; ++i)
			{
				auto &vertex = v[i];
				for (U32 j = 0; j < 3; ++j)
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
		std::vector<U32> indices;
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
	for (U32 i = 0; i < triangleCount; ++i)
	{
		auto i0 = triangleIndices[i].vertexIndex[0];
		auto i1 = triangleIndices[i].vertexIndex[1];
		auto i2 = triangleIndices[i].vertexIndex[2];

		NavGraphTriangle t;
		t.v[0] = vertices[i0];
		t.v[1] = vertices[i1];
		t.v[2] = vertices[i2];
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

	U32 connectionCount = 0;
	// find triangles that share edges
	for (U32 j = 0; j < triangleCount; ++j)
	{
		auto &it = triangles[j];

		U8 sharedCount = 0;
		for (U32 i = 0; i < triangleCount; ++i)
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
	U32 connectionOffset = 0;

	m_nodeCount = triangleCount;
#if _VX_EDITOR
	VX_UNREFERENCED_PARAMETER(pAllocator);
	m_nodes.reserve(m_nodeCount);
#else
	m_nodes = (NavNode*)pAllocator->allocate(sizeof(NavNode) * m_nodeCount, __alignof(NavNode));
#endif

	for (U32 i = 0; i < triangleCount; ++i)
	{
		auto &triangle = triangles[i];

		NavNode node;
		node.m_connectionCount = triangle.sharedEdgesCount;
		node.m_connectionOffset = connectionOffset;
		node.m_position = triangle.c;

#if _VX_EDITOR
		m_nodes.push_back(node);
#else
		m_nodes[i] = node;
#endif

		for (U32 j = 0; j < triangle.sharedEdgesCount; ++j)
		{
			auto it = triangle.sharedEdges[j];

			NavConnection connection;
			connection.m_fromNode = i;
			connection.m_toNode = it;

			auto &toTriangle = triangles[it];

			connection.m_cost = vx::distance(triangle.c, toTriangle.c);

			connections[connectionOffset++] = connection;
		}
	}

	VX_ASSERT(connectionCount == connectionOffset);

#if _VX_EDITOR
	m_connections.reserve(connectionCount);
	for(U32 i = 0;i < connectionCount; ++i)
	{
		m_connections.push_back(connections[i]);
	}
#else
	m_connections = (NavConnection*)pAllocator->allocate(sizeof(NavConnection) * connectionCount, __alignof(NavConnection));
	vx::memcpy(m_connections, connections, connectionCount);
#endif
	m_connectionCount = connectionCount;
}

void NavGraph::shutdown(vx::StackAllocator* pAllocator)
{
#if _VX_EDITOR
	VX_UNREFERENCED_PARAMETER(pAllocator);
	m_nodes.clear();
	m_connections.clear();
#else
	pAllocator->rangeDestroy(m_connections, m_connections + m_connectionCount);
	pAllocator->rangeDestroy(m_nodes, m_nodes + m_nodeCount);
#endif
}

U32 NavGraph::getClosestNode(const vx::float3 &position) const
{
	F32 distance = FLT_MAX;
	U32 nodeIndex = -1;
	for (U32 i = 0; i < m_nodeCount; ++i)
	{
		auto &node = m_nodes[i];

		auto currentDistance = vx::distance2(position, node.m_position);

		if (currentDistance < distance)
		{
			distance = currentDistance;
			nodeIndex = i;
		}
	}

	return nodeIndex;
}

U32 NavGraph::getFarestNode(const vx::float3 &position) const
{
	F32 distance = -FLT_MAX;
	U32 nodeIndex = -1;
	for (U32 i = 0; i < m_nodeCount; ++i)
	{
		auto &node = m_nodes[i];

		auto currentDistance = vx::distance2(position, node.m_position);

		if (currentDistance > distance)
		{
			distance = currentDistance;
			nodeIndex = i;
		}
	}

	return nodeIndex;
}

const NavNode& NavGraph::getNode(U32 i) const
{
	return m_nodes[i];
}