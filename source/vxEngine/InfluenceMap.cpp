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
#include "InfluenceMap.h"
#include "NavMesh.h"
#include "NavGraph.h"
#include "NavNode.h"
#include "NavMeshTriangle.h"
#include <vxLib/Container/sorted_vector.h>
#include <vxLib/algorithm.h>
#include <vector>

namespace
{
	struct Triangle_SIMD
	{
		__m128 v[3];
	};

	struct AABB_POINT
	{
		static bool VX_CALLCONV contains(const __m128 vmin, const __m128 vmax, const __m128 p)
		{
			// only need to consider the first three values
			const int resultMask = 1 << 0 | 1 << 1 | 1 << 2;

			auto cmp1 = _mm_cmpge_ps(p, vmin);
			auto cmp2 = _mm_cmple_ps(p, vmax);

			auto m1 = _mm_movemask_ps(cmp1);
			auto m2 = _mm_movemask_ps(cmp2);

			auto m = (m1 & m2) & resultMask;

			return (m == resultMask);
		}
	};

	struct BuildInfluenceCell
	{
		std::vector<u32> indices;
		f32 totalArea;
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

	void addTrianglesThatShareEdgesToCell(f32 minArea, const NavMeshTriangle* triangles, vx::sorted_vector<vx::float3, u32, CompareFloat3>* sortedTriangles, BuildInfluenceCell* currentCell)
	{
		for (u32 k = 0; k < currentCell->indices.size(); ++k)
		{
			auto index = currentCell->indices[k];
			auto &currentTriangle = triangles[index];

			for (u32 i = 0; i < currentTriangle.m_count; ++i)
			{
				auto sharedIndex = currentTriangle.m_sharedEdges[i];
				auto &sharedTriangle = triangles[sharedIndex];

				auto it = sortedTriangles->find(sharedTriangle.m_triangle.getCentroid());
				if (it != sortedTriangles->end())
				{
					currentCell->indices.push_back(*it);
					currentCell->totalArea += sharedTriangle.m_triangle.getArea();

					sortedTriangles->erase(it);
				}
			}

			if (currentCell->totalArea >= minArea)
				break;
		}
	}

	void createCellsNew(const NavMeshTriangle* triangles, u32 triangleCount, f32 minArea, std::vector<BuildInfluenceCell>* buildCells)
	{
		vx::sorted_vector<vx::float3, u32, CompareFloat3> sortedTriangles;
		sortedTriangles.reserve(triangleCount);

		for (u32 i = 0; i < triangleCount; ++i)
		{
			auto centroid = triangles[i].m_triangle.getCentroid();
			sortedTriangles.insert(centroid, i);
		}

		BuildInfluenceCell currentCell;
		currentCell.totalArea = 0.0f;

		auto currentIndex = sortedTriangles.back();
		sortedTriangles.pop_back();

		auto &currentTriangle = triangles[currentIndex];
		currentCell.indices.push_back(currentIndex);
		currentCell.totalArea += currentTriangle.m_triangle.getArea();

		while (!sortedTriangles.empty())
		{
			addTrianglesThatShareEdgesToCell(minArea, triangles, &sortedTriangles, &currentCell);

			buildCells->push_back(currentCell);
			currentCell.indices.clear();
			currentCell.totalArea = 0.0f;

			if (!sortedTriangles.empty())
			{
				currentIndex = sortedTriangles.back();
				auto &currentTriangle = triangles[currentIndex];

				sortedTriangles.pop_back();

				currentCell.indices.push_back(currentIndex);
				currentCell.totalArea += currentTriangle.m_triangle.getArea();
			}
		}
	}

	bool cellsShareEdge(const BuildInfluenceCell &currentCell, const BuildInfluenceCell &otherCell, const NavMeshTriangle* triangles)
	{
		for (auto &currentIndex : currentCell.indices)
		{
			auto &currentTriangle = triangles[currentIndex];
			for (auto &otherIndex : otherCell.indices)
			{
				auto &otherTriangle = triangles[otherIndex];
				if (currentTriangle.sharesEdge(otherTriangle))
				{
					return true;
				}
			}
		}

		return false;
	}

	BuildInfluenceCell* findCellThatSharesEdge(const BuildInfluenceCell &currentCell, const NavMeshTriangle* triangles, std::vector<BuildInfluenceCell>* cells)
	{
		BuildInfluenceCell* sharedCell = nullptr;

		for (auto &otherCell : *cells)
		{
			if (cellsShareEdge(currentCell, otherCell, triangles))
			{
				sharedCell = &otherCell;
			}
		}

		return sharedCell;
	}

	void mergCellWithExistingOrAdd(const BuildInfluenceCell &currentCell, const NavMeshTriangle* triangles, std::vector<BuildInfluenceCell>* cells)
	{
		auto sharedCell = findCellThatSharesEdge(currentCell, triangles, cells);

		if (sharedCell != nullptr)
		{
			for (auto &currentIndex : currentCell.indices)
			{
				sharedCell->indices.push_back(currentIndex);
			}

			sharedCell->totalArea += currentCell.totalArea;
		}
		else
		{
			cells->push_back(currentCell);
		}
	}

	void addOrMergeCells(f32 minArea, const std::vector<BuildInfluenceCell> &buildCells, const NavMeshTriangle* triangles, std::vector<BuildInfluenceCell>* finalCells)
	{
		for (auto &currentCell : buildCells)
		{
			if (currentCell.totalArea < minArea)
			{
				mergCellWithExistingOrAdd(currentCell, triangles, finalCells);
			}
			else
			{
				finalCells->push_back(currentCell);
			}
		}
	}

	void buildCellsNew(const NavMeshTriangle* triangles, u32 triangleCount, std::vector<BuildInfluenceCell>* finalCells)
	{
		const f32 minArea = 20.0f;

		std::vector<BuildInfluenceCell> buildCells;
		createCellsNew(triangles, triangleCount, minArea, &buildCells);

		addOrMergeCells(minArea, buildCells, triangles, finalCells);
	}
}

InfluenceMap::InfluenceMap()
	:m_cellCount()
{

}

InfluenceMap::~InfluenceMap()
{

}

void InfluenceMap::initialize(const NavMesh &navMesh)
{
	auto triangles = navMesh.getNavMeshTriangles();
	auto triangleCount = navMesh.getTriangleCount();

	std::vector<BuildInfluenceCell> finalCells;
	buildCellsNew(triangles, triangleCount, &finalCells);

	m_triangles = vx::make_unique<Triangle[]>(triangleCount);
	m_cells = vx::make_unique<InfluenceCell[]>(finalCells.size());
	m_cellCount = finalCells.size();

	u32 triangleIndex = 0;
	u32 cellIndex = 0;
	u32 triangleOffset = 0;
	for (auto &buildCell : finalCells)
	{
		m_cells[cellIndex].triangleCount = buildCell.indices.size();
		m_cells[cellIndex].triangleOffset = triangleOffset;
		m_cells[cellIndex].totalArea = buildCell.totalArea;
		++cellIndex;

		for (auto &index : buildCell.indices)
		{
			m_triangles[triangleIndex++] = triangles[index].m_triangle;
			++triangleOffset;
		}
	}
}

const InfluenceCell* InfluenceMap::getCellsNew() const
{
	return m_cells.get();
}

const Triangle* InfluenceMap::getTriangles() const
{
	return m_triangles.get();
}

u32 InfluenceMap::getCellsNewCount() const
{
	return m_cellCount;
}