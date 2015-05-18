#pragma once
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

class NavMesh;

#include <vxLib/memory.h>
#include "Triangle.h"
#include "AABB.h"

struct InfluenceCell
{
	u32 triangleCount;
	u32 triangleOffset;
	f32 totalArea;
};

class InfluenceMap
{
	std::unique_ptr<InfluenceCell[]> m_cells;
	std::unique_ptr<AABB[]> m_bounds;
	std::unique_ptr<Triangle[]> m_triangles;
	u32 m_cellCount;

public:
	InfluenceMap();
	~InfluenceMap();

	void initialize(const NavMesh &navMesh);

	const InfluenceCell* getCells() const;
	const Triangle* getTriangles() const;
	const AABB* getBounds() const;
	u32 getCellCount() const;

	bool sharesEdge(const InfluenceCell &a, const InfluenceCell &b) const;
};