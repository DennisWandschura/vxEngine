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
	struct Triangle
	{
		vx::float3 v[3];
		vx::float3 c;
		U16 sharedEdges[3];
		U8 sharedEdgesCount{ 0 };

		bool sharesEdge(const Triangle &other)
		{
			if (this == &other)
				return false;

			U8 count = 0;

			for (U32 i = 0; i < 3; ++i)
			{
				auto &current = v[i];
				for (U32 j = 0; j < 3; ++j)
				{
					auto &otherCurrent = other.v[j];

					auto result = (current.x == otherCurrent.x) & (current.y == otherCurrent.y) & (current.z == otherCurrent.z);

					count += result;

					//if (current.x == other.v[j].x && current.y == other.v[j].y && current.z == other.v[j].z)
					//	++count;
				}
			}

			/*if (v[0].x == other.v[0].x && v[0].y == other.v[0].y && v[0].z == other.v[0].z)
				++count;

			if (v[1].x == other.v[1].x && v[1].y == other.v[1].y && v[1].z == other.v[1].z)
				++count;

			if (v[2].x == other.v[2].x && v[2].y == other.v[2].y && v[2].z == other.v[2].z)
				++count;*/

			return (count > 1);
		}
	};

	struct BuildCell
	{
		std::vector<U32> indices;
	};
}

NavGraph::NavGraph()
{

}

NavGraph::~NavGraph()
{

}

void NavGraph::initialize(const NavMesh &navMesh, vx::StackAllocator* pAllocator, vx::StackAllocator* pAllocatorScratch)
{
	auto vertices = navMesh.getVertices();
	auto indices = navMesh.getIndices();

	auto iCount = navMesh.getIndexCount();

	VX_ASSERT(iCount % 3 == 0);

	auto triangleCount = iCount / 3;

	//std::vector<Triangle> triangles;
	//triangles.reserve(triangleCount);
	auto scratchMarker = pAllocatorScratch->getMarker();

	SCOPE_EXIT
	{
		pAllocatorScratch->clear(scratchMarker);
	};

	auto triangles = (Triangle*)pAllocatorScratch->allocate(sizeof(Triangle) * triangleCount, __alignof(Triangle));

	// bounds of centroids
	AABB bounds;
	// create triangles
	for (U32 i = 0, j = 0; i < iCount; i += 3, ++j)
	{
		auto i0 = indices[i];
		auto i1 = indices[i + 1];
		auto i2 = indices[i + 2];

		Triangle t;
		t.v[0] = vertices[i0];
		t.v[1] = vertices[i1];
		t.v[2] = vertices[i2];
		t.c = (t.v[0] + t.v[1] + t.v[2]) / 3.0f;

		bounds = bounds.Union(t.c);

		triangles[j] = t;
	}

	//vx::float3 center = (bounds.max + bounds.min) * 0.5f;
//	auto size = (bounds.max - center) + (center - bounds.min);
//	vx::uint3 cellCount = (size + vx::float3(s_cellSize)) / vx::float3(s_cellSize);
	//const F32 cellHalfSize = s_cellSize / 2.0f;

	//auto index = Cell::getCellIndex(center, center, cellCount, s_cellSize);

	// sort triangles by position
	std::sort(triangles, triangles + triangleCount, [](const Triangle &lhs, const Triangle &rhs)
	{
		return (
			(lhs.c.x < rhs.c.x) ||
			(lhs.c.x == rhs.c.x && lhs.c.y < rhs.c.y) ||
			(lhs.c.x == rhs.c.x && lhs.c.y == rhs.c.y && lhs.c.z < rhs.c.z)
			);
	});

	/*auto totalCellCount = cellCount.x * cellCount.y * cellCount.z;
	std::vector<BuildCell> cells;
	cells.reserve(totalCellCount);

	for (U32 i = 0; i < totalCellCount; ++i)
	{
		cells.push_back(BuildCell());

		// get 3d index
		U32 z = i / (cellCount.x * cellCount.y);
		U32 y = (i - (z*cellCount.x*cellCount.y)) / cellCount.x;
		U32 x = i - y * cellCount.x - z * cellCount.x * cellCount.y;

		auto tmp = vx::float3(x, y, z);

		// assuming origin in bottom left
		AABB cellBounds;
		cellBounds.min = tmp * vx::float3(s_cellSize);
		//cellBounds.max = (tmp + vx::float3(1.0f)) * vx::float3(s_cellSize);

		// offset by halfsize
		auto offset = cellHalfSize * vx::float3(cellCount);
		cellBounds.min = cellBounds.min - offset;
		cellBounds.max = cellBounds.min + vx::float3(s_cellSize);

		auto dim = (cellBounds.max - cellBounds.min) * 0.5f;
		auto cc = (cellBounds.max + cellBounds.min) * 0.5f + center;

		cellBounds.min = cc - dim;
		cellBounds.max = cc + dim;

		auto &cell = cells[i];

		// iterate over trangles
		for (U32 j = 0; j < triangleCount; ++j)
		{
			if (cellBounds.contains(triangles[j].c))
			{
				cell.indices.push_back(j);
			}
		}

		int k = 0;
		++k;
	}*/
	//auto trianglesGridSorted = (Triangle*)pAllocatorScratch->allocate(sizeof(Triangle) * triangleCount, __alignof(Triangle));


	U32 connectionCount = 0;
	// find triangles that share edges
	for (U32 j = 0; j < triangleCount; ++j)
	{
		auto &it = triangles[j];
		//std::vector<U16> edges;
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
	m_nodes = (NavNode*)pAllocator->allocate(sizeof(NavNode) * m_nodeCount, __alignof(NavNode));

	for (U32 i = 0; i < triangleCount; ++i)
	{
		auto &triangle = triangles[i];

		m_nodes[i].m_connectionCount = triangle.sharedEdgesCount;
		m_nodes[i].m_connectionOffset = connectionOffset;
		m_nodes[i].m_position = triangle.c;

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

	m_connections = (NavConnection*)pAllocator->allocate(sizeof(NavConnection) * connectionCount, __alignof(NavConnection));
	vx::memcpy(m_connections, connections, connectionCount);
	m_connectionCount = connectionCount;
}

void NavGraph::shutdown(vx::StackAllocator* pAllocator)
{
	pAllocator->rangeDestroy(m_connections, m_connections + m_connectionCount);
	pAllocator->rangeDestroy(m_nodes, m_nodes + m_nodeCount);
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