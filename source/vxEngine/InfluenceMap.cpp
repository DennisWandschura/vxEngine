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
#include <vxEngineLib/NavMesh.h>
#include <vxEngineLib/NavMeshTriangle.h>
#include "NavGraph.h"
#include "NavNode.h"
#include <vxLib/Container/sorted_vector.h>
#include <vxLib/algorithm.h>
#include <vector>
#include <vxEngineLib/Waypoint.h>

namespace InfluenceMapCpp
{
	struct BuildCell
	{
		std::vector<u32> indices;
		std::vector<Waypoint> waypoints;
		f32 totalArea;
	};

	struct InfluenceMapCompareFloat3
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

	void addTrianglesThatShareEdgesToCell(f32 minArea, const NavMeshTriangle* triangles, vx::sorted_vector<vx::float3, u32, InfluenceMapCompareFloat3>* sortedTriangles, BuildCell* currentCell)
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

	void createCellsNew(const NavMeshTriangle* triangles, u32 triangleCount, f32 minArea, std::vector<BuildCell>* buildCells)
	{
		vx::sorted_vector<vx::float3, u32, InfluenceMapCompareFloat3> sortedTriangles;
		sortedTriangles.reserve(triangleCount);

		for (u32 i = 0; i < triangleCount; ++i)
		{
			auto centroid = triangles[i].m_triangle.getCentroid();
			sortedTriangles.insert(centroid, i);
		}

		BuildCell currentCell;
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

	bool cellsShareEdge(const BuildCell &currentCell, const BuildCell &otherCell, const NavMeshTriangle* triangles)
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

	BuildCell* findCellThatSharesEdge(const BuildCell &currentCell, const NavMeshTriangle* triangles, std::vector<BuildCell>* cells)
	{
		BuildCell* sharedCell = nullptr;

		for (auto &otherCell : *cells)
		{
			if (cellsShareEdge(currentCell, otherCell, triangles))
			{
				sharedCell = &otherCell;
			}
		}

		return sharedCell;
	}

	void mergCellWithExistingOrAdd(const BuildCell &currentCell, const NavMeshTriangle* triangles, std::vector<BuildCell>* cells)
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

	void addOrMergeCells(f32 minArea, const std::vector<BuildCell> &buildCells, const NavMeshTriangle* triangles, std::vector<BuildCell>* finalCells)
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

	void buildCellsNew(const NavMeshTriangle* triangles, u32 triangleCount, std::vector<BuildCell>* finalCells)
	{
		const f32 minArea = 20.0f;

		std::vector<BuildCell> buildCells;
		createCellsNew(triangles, triangleCount, minArea, &buildCells);

		addOrMergeCells(minArea, buildCells, triangles, finalCells);
	}

	void createInfluenceCellBounds(const BuildCell &cell, const NavMeshTriangle* triangles, AABB* bounds)
	{
		for (auto &it : cell.indices)
		{
			auto &triangle = triangles[it];

			*bounds = AABB::merge(*bounds, triangle.m_triangle[0]);
			*bounds = AABB::merge(*bounds, triangle.m_triangle[1]);
			*bounds = AABB::merge(*bounds, triangle.m_triangle[2]);
		}
	}

	bool addWaypoint(const Waypoint &waypoint, const NavMeshTriangle* triangles, BuildCell* cell)
	{
		bool result = false;
		for (auto &it : cell->indices)
		{
			auto &triangle = triangles[it];

			if (triangle.m_triangle.contains(waypoint.position))
			{
				cell->waypoints.push_back(waypoint);
				result = true;
				break;
			}
		}

		return result;
	}
}

InfluenceMap::InfluenceMap()
	:m_cellCount(0),
	m_waypointCount(0)
{

}

InfluenceMap::~InfluenceMap()
{

}

void InfluenceMap::initialize(const NavMesh &navMesh, const Waypoint* waypoints, u32 count)
{
	if (count == 0)
		return;

	auto triangles = navMesh.getNavMeshTriangles();
	auto triangleCount = navMesh.getTriangleCount();

	std::vector<InfluenceMapCpp::BuildCell> finalCells;
	InfluenceMapCpp::buildCellsNew(triangles, triangleCount, &finalCells);

	u32 currentWaypointIndex = 0;

	while (currentWaypointIndex != count)
	{
		for (auto &buildCell : finalCells)
		{
			for (u32 i = currentWaypointIndex; i < count; ++i)
			{
				auto &waypoint = waypoints[currentWaypointIndex];
				if (InfluenceMapCpp::addWaypoint(waypoint, triangles, &buildCell))
				{
					++currentWaypointIndex;
				}
			}
		}
	}

	m_triangles = vx::make_unique<Triangle[]>(triangleCount);

	m_cellCount = finalCells.size();
	m_cells = vx::make_unique<InfluenceCell[]>(m_cellCount);
	m_bounds = vx::make_unique<AABB[]>(m_cellCount);
	m_waypoints = vx::make_unique<Waypoint[]>(count);
	m_waypointCount = count;

	u32 triangleIndex = 0;
	u32 cellIndex = 0;
	u32 triangleOffset = 0;
	currentWaypointIndex = 0;
	for (auto &buildCell : finalCells)
	{
		InfluenceMapCpp::createInfluenceCellBounds(buildCell, triangles, &m_bounds[cellIndex]);
		m_bounds[cellIndex].min.y -= 0.1f;
		m_bounds[cellIndex].max.y += 0.1f;

		u16 waypointCount = buildCell.waypoints.size();
		u16 waypointOffset = currentWaypointIndex;
		for (auto &waypoint : buildCell.waypoints)
		{
			m_waypoints[currentWaypointIndex] = waypoint;
			++currentWaypointIndex;
		}

		m_cells[cellIndex].triangleCount = buildCell.indices.size();
		m_cells[cellIndex].triangleOffset = triangleOffset;
		m_cells[cellIndex].totalArea = buildCell.totalArea;
		m_cells[cellIndex].waypointCount = waypointCount;
		m_cells[cellIndex].waypointOffset = waypointOffset;
		++cellIndex;

		for (auto &index : buildCell.indices)
		{
			m_triangles[triangleIndex++] = triangles[index].m_triangle;
			++triangleOffset;
		}
	}
}

const InfluenceCell* InfluenceMap::getCells() const
{
	return m_cells.get();
}

const Triangle* InfluenceMap::getTriangles() const
{
	return m_triangles.get();
}

const AABB* InfluenceMap::getBounds() const
{
	return m_bounds.get();
}

u32 InfluenceMap::getCellCount() const
{
	return m_cellCount;
}

const Waypoint* InfluenceMap::getWaypoints() const
{
	return m_waypoints.get();
}

u32 InfluenceMap::getWaypointCount() const
{
	return m_waypointCount;
}

bool InfluenceMap::sharesEdge(const InfluenceCell &a, const InfluenceCell &b) const
{
	u32 offsetA = a.triangleOffset;
	u32 offsetB = b.triangleOffset;

	for (u32 i = 0; i < a.triangleCount; ++i)
	{
		auto &triangleA = m_triangles[i + offsetA];

		for (u32 j = 0; j < b.triangleCount; ++j)
		{
			auto &triangleB = m_triangles[j + offsetB];

			if (triangleA.sharesEdge(triangleB))
				return true;
		}
	}

	return false;
}