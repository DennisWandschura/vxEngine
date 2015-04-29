#include "NavMeshTriangle.h"
#include <cstdio>

bool NavMeshTriangle::sharesEdge(const NavMeshTriangle &other) const
{
	U8 sharedCount = 0;

	if (m_triangle[0].x == other.m_triangle.m_points[0].x &&
		m_triangle[0].y == other.m_triangle.m_points[0].y &&
		m_triangle[0].z == other.m_triangle.m_points[0].z)
		++sharedCount;

	if (m_triangle[1].x == other.m_triangle.m_points[1].x &&
		m_triangle[1].y == other.m_triangle.m_points[1].y &&
		m_triangle[1].z == other.m_triangle.m_points[1].z)
		++sharedCount;

	if (m_triangle[2].x == other.m_triangle.m_points[2].x &&
		m_triangle[2].y == other.m_triangle.m_points[2].y &&
		m_triangle[2].z == other.m_triangle.m_points[2].z)
		++sharedCount;

	return sharedCount > 1;
}