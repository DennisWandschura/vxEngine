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
#include "NavMeshTriangle.h"
#include <cstdio>

bool NavMeshTriangle::sharesEdge(const NavMeshTriangle &other) const
{
	u8 sharedCount = 0;

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