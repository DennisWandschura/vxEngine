#pragma once

#include "Triangle.h"

struct NavMeshTriangle
{
	Triangle m_triangle;
	U16 m_sharedEdges[3];
	U16 m_count;

	bool sharesEdge(const NavMeshTriangle &other) const;
};