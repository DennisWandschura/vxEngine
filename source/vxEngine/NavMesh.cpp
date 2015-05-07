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
#include "NavMesh.h"
#include "utility.h"
#include "File.h"
#include "NavMeshTriangle.h"
#include <algorithm>

/*namespace YAML
{
	template<>
	struct convert < AABB >
	{
		static Node encode(const AABB &rhs)
		{
			Node n;
			n["min"] = rhs.min;
			n["max"] = rhs.max;

			return n;
		}

		static bool decode(const Node &node, AABB &v)
		{
			if (node.size() != 2 && !node.IsSequence())
				return false;

			v.min = node["min"].as<vx::float3>();
			v.max = node["max"].as<vx::float3>();

			return true;
		}
	};

	template<>
	struct convert < TriangleIndices >
	{
		static Node encode(const TriangleIndices &rhs)
		{
			Node n;
			n.push_back(rhs.vertexIndex[0]);
			n.push_back(rhs.vertexIndex[1]);
			n.push_back(rhs.vertexIndex[2]);

			return n;
		}

		static bool decode(const Node &node, TriangleIndices &v)
		{
			if (node.size() != 3)
				return false;

			v.vertexIndex[0] = node[0].as<U16>();
			v.vertexIndex[1] = node[1].as<U16>();
			v.vertexIndex[2] = node[2].as<U16>();

			return true;
		}
	};
}*/

NavMesh::NavMesh()
	:m_navMeshTriangles(),
	m_vertices(),
	m_triangleIndices(),
#if _VX_EDITOR
	m_vertexBounds(),
#endif
	m_bounds(),
	m_vertexCount(0),
	m_triangleCount(0)
{
}

NavMesh::NavMesh(NavMesh &&rhs)
	: m_navMeshTriangles(std::move(rhs.m_navMeshTriangles)),
	m_vertices(std::move(rhs.m_vertices)),
	m_triangleIndices(std::move(rhs.m_triangleIndices)),
#if _VX_EDITOR
	m_vertexBounds(std::move(rhs.m_vertexBounds)),
#endif
	m_bounds(std::move(rhs.m_bounds)),
	m_vertexCount(rhs.m_vertexCount),
	m_triangleCount(rhs.m_triangleCount)
{
}

NavMesh::~NavMesh()
{
}

NavMesh& NavMesh::operator = (NavMesh &&rhs)
{
	if (this != &rhs)
	{
		std::swap(m_navMeshTriangles, rhs.m_navMeshTriangles);
		std::swap(m_vertices, rhs.m_vertices);
		std::swap(m_triangleIndices, rhs.m_triangleIndices);
#if _VX_EDITOR
		std::swap(m_vertexBounds, rhs.m_vertexBounds);
#endif
		std::swap(m_bounds, rhs.m_bounds);
		std::swap(m_vertexCount, rhs.m_vertexCount);
		std::swap(m_triangleCount, rhs.m_triangleCount);
	}

	return *this;
}

void NavMesh::copyTo(NavMesh* other) const
{
#if _VX_EDITOR
	other->m_vertices.clear();
	if (other->m_vertexCount < m_vertexCount)
	{
		other->m_vertices.reserve(m_vertexCount);
	}

	for (auto &it : m_vertices)
	{
		other->m_vertices.push_back(it);
	}

	other->m_triangleIndices.clear();
	if (other->m_triangleCount < m_triangleCount)
	{
		other->m_navMeshTriangles = std::make_unique<NavMeshTriangle[]>(m_triangleCount);
		other->m_triangleIndices.reserve(m_triangleCount);
	}

	for (auto &it : m_triangleIndices)
	{
		other->m_triangleIndices.push_back(it);
	}

	other->m_vertexBounds.clear();
	other->m_vertexBounds.reserve(m_vertexBounds.size());
	for (auto &it : m_vertexBounds)
	{
		other->m_vertexBounds.push_back(it);
	}
#else
	if (other->m_vertexCount < m_vertexCount)
	{
		other->m_vertices = std::make_unique<vx::float3[]>(m_vertexCount);
	}
	vx::memcpy(other->m_vertices.get(), m_vertices.get(), m_vertexCount);

	if (other->m_triangleCount < m_triangleCount)
	{
		other->m_navMeshTriangles = std::make_unique<NavMeshTriangle[]>(m_triangleCount);
		other->m_triangleIndices = std::make_unique<TriangleIndices[]>(m_triangleCount);
	}
	vx::memcpy(other->m_triangleIndices.get(), m_triangleIndices.get(), m_triangleCount);
#endif

	vx::memcpy(other->m_navMeshTriangles.get(), m_navMeshTriangles.get(), m_triangleCount);

	other->m_bounds = m_bounds;
	other->m_vertexCount = m_vertexCount;
	other->m_triangleCount = m_triangleCount;
}

/*void NavMesh::loadFromYAML(const YAML::Node &node)
{
	auto vertices = node["vertices"].as<std::vector<vx::float3>>();
	auto triangleIndices = node["indices"].as<std::vector<TriangleIndices>>();

	m_vertexCount = vertices.size();
	m_triangleCount = triangleIndices.size();

	m_bounds = node["bounds"].as<AABB>();

#if _VX_EDITOR
	m_vertices = std::move(vertices);

	for (U32 i = 0; i < m_triangleCount; ++i)
	{
		m_triangleIndices.push_back(triangleIndices[i]);
	}

	m_vertexBounds.reserve(m_vertexCount);
	for (auto &it : m_vertices)
	{
		AABB bounds;
		bounds.min = it - vx::float3(0.1f);
		bounds.max = it + vx::float3(0.1f);

		m_vertexBounds.push_back(bounds);
	}
#else
	m_vertices = std::make_unique<vx::float3[]>(m_vertexCount);
	m_triangleIndices = std::make_unique<TriangleIndices[]>(m_triangleCount);

	vx::memcpy(m_vertices.get(), vertices.data(), m_vertexCount);
	vx::memcpy(m_triangleIndices.get(), triangleIndices.data(), m_triangleCount);
#endif

	m_navMeshTriangles = createNavMeshTriangles();
}*/

void NavMesh::saveToFile(File *file) const
{
	file->write(m_vertexCount);
	file->write(m_triangleCount);
	file->write(m_bounds);

#if _VX_EDITOR
	file->write(m_vertices.data(), m_vertexCount);
	file->write(m_triangleIndices.data(), m_triangleCount);
#else
	file->write(m_vertices.get(), m_vertexCount);
	file->write(m_triangleIndices.get(), m_triangleCount);
#endif
}

/*void NavMesh::saveToYAML(YAML::Node &node) const
{
#if _VX_EDITOR
	node["vertices"] = m_vertices;
	node["indices"] = m_triangleIndices;
#else
	auto vertices = std::vector<vx::float3>(m_vertexCount);
	vx::memcpy(vertices.data(), m_vertices.get(), m_vertexCount);

	auto triangles = std::vector<TriangleIndices>(m_triangleCount);
	vx::memcpy(triangles.data(), m_triangleIndices.get(), m_triangleCount);

	node["vertices"] = vertices;
	node["indices"] = triangles;
#endif
	node["bounds"] = m_bounds;
}*/

const U8* NavMesh::load(const U8 *ptr)
{
	ptr = vx::read(m_vertexCount, ptr);
	ptr = vx::read(m_triangleCount, ptr);
	ptr = vx::read(m_bounds, ptr);

	#warning Please use a custom allocator !

#if _VX_EDITOR
	m_vertices = std::vector<vx::float3>(m_vertexCount);
	m_triangleIndices = std::vector<TriangleIndices>(m_triangleCount);

	ptr = vx::read(m_vertices.data(), ptr, m_vertexCount);
	ptr = vx::read(m_triangleIndices.data(), ptr, m_triangleCount);

	m_vertexBounds.reserve(m_vertexCount);
	for (auto &it : m_vertices)
	{
		AABB bounds;
		bounds.min = it - vx::float3(0.1f);
		bounds.max = it + vx::float3(0.1f);

		m_vertexBounds.push_back(bounds);
	}
#else
	m_vertices = std::make_unique<vx::float3[]>(m_vertexCount);
	m_triangleIndices = std::make_unique<TriangleIndices[]>(m_triangleCount);

	ptr = vx::read(m_vertices.get(), ptr, m_vertexCount);
	ptr = vx::read(m_triangleIndices.get(), ptr, m_triangleCount);
#endif

	m_navMeshTriangles = createNavMeshTriangles();

	return ptr;
}
void NavMesh::addVertex(const vx::float3 &vertex)
{
#if _VX_EDITOR
	m_vertices.push_back(vertex);
	++m_vertexCount;

	m_bounds = AABB::merge(m_bounds, vertex);

	AABB bounds;
	bounds.min = vertex - vx::float3(0.1f);
	bounds.max = vertex + vx::float3(0.1f);

	m_vertexBounds.push_back(bounds);
#else
	VX_UNREFERENCED_PARAMETER(vertex);
#endif
}

void NavMesh::findAndEraseVertexFromIndices(U16 vertexIndex)
{
#if _VX_EDITOR
	auto it = m_triangleIndices.begin();
	while (it != m_triangleIndices.end())
	{
		it = std::find_if(m_triangleIndices.begin(), m_triangleIndices.end(), [vertexIndex](const TriangleIndices &o)
		{
			bool result = o.vertexIndex[0] == vertexIndex ||
				o.vertexIndex[1] == vertexIndex ||
				o.vertexIndex[2] == vertexIndex;

			return result;
		});

		if (it != m_triangleIndices.end())
		{
			it = m_triangleIndices.erase(it);
			--m_triangleCount;
		}
	}
#else
	VX_UNREFERENCED_PARAMETER(vertexIndex);
#endif
}

void NavMesh::fixVertexIndicesAfterErasedVertex(U16 erasedVertexIndex)
{
#if _VX_EDITOR
	for (auto &it : m_triangleIndices)
	{
		if (it.vertexIndex[0] >= erasedVertexIndex)
		{
			--it.vertexIndex[0];
		}

		if (it.vertexIndex[1] >= erasedVertexIndex)
		{
			--it.vertexIndex[1];
		}

		if (it.vertexIndex[2] >= erasedVertexIndex)
		{
			--it.vertexIndex[2];
		}
	}
#else
	VX_UNREFERENCED_PARAMETER(erasedVertexIndex);
#endif
}

std::unique_ptr<NavMeshTriangle[]> NavMesh::createNavMeshTriangles()
{
	auto ptr = std::make_unique<NavMeshTriangle[]>(m_triangleCount);
	for (U32 i = 0; i < m_triangleCount; ++i)
	{
		auto &it = m_triangleIndices[i];

		ptr[i].m_triangle[0] = m_vertices[it.vertexIndex[0]];
		ptr[i].m_triangle[1] = m_vertices[it.vertexIndex[1]];
		ptr[i].m_triangle[2] = m_vertices[it.vertexIndex[2]];
		ptr[i].m_count = 0;
	}

	for (U32 j = 0; j < m_triangleCount; ++j)
	{
		auto &it = ptr[j];

		for (U32 i = 0; i < m_triangleCount; ++i)
		{
			auto &other = ptr[i];
			if (i != j)
			{
				if (it.sharesEdge(other))
				{
					it.m_sharedEdges[it.m_count++] = i;
				}
			}
		}
	}

	return ptr;
}

void NavMesh::deleteVertex(U32 index)
{
#if _VX_EDITOR
	m_vertices.erase(m_vertices.begin() + index);
	m_vertexBounds.erase(m_vertexBounds.begin() + index);
	--m_vertexCount;

	m_bounds = AABB();
	for (auto &it : m_vertices)
	{
		m_bounds = AABB::merge(m_bounds, it);
	}
	
	findAndEraseVertexFromIndices(index);

	fixVertexIndicesAfterErasedVertex(index);

	m_navMeshTriangles = createNavMeshTriangles();
#else
	VX_UNREFERENCED_PARAMETER(index);
#endif
}

void NavMesh::addTriangle(const U32(&selectedIndices)[3])
{
#if _VX_EDITOR
	auto v0 = m_vertices[selectedIndices[0]];
	auto v1 = m_vertices[selectedIndices[1]];
	auto v2 = m_vertices[selectedIndices[2]];

	TriangleIndices triangleIndices;
	triangleIndices.vertexIndex[0] = selectedIndices[0];
	triangleIndices.vertexIndex[1] = selectedIndices[1];
	triangleIndices.vertexIndex[2] = selectedIndices[2];

	if (isCCW(v0, v1, v2))
	{
		m_triangleIndices.push_back(triangleIndices);

		++m_triangleCount;

		m_navMeshTriangles = createNavMeshTriangles();
	}
	else
	{
		printf("not ccw !\n");
	}
#else
	VX_UNREFERENCED_PARAMETER(selectedIndices);
#endif
}

void NavMesh::setVertexPosition(U32 i, const vx::float3 &position)
{
#if _VX_EDITOR
	m_vertices[i] = position;
	m_vertexBounds[i].min = position - vx::float3(0.1f);
	m_vertexBounds[i].max = position + vx::float3(0.1f);

	m_bounds = AABB::merge(m_bounds, position);

	m_navMeshTriangles = createNavMeshTriangles();
#else
	VX_UNREFERENCED_PARAMETER(i);
	VX_UNREFERENCED_PARAMETER(position);
#endif
}

bool NavMesh::isValid() const
{
	bool result = true;
	/*for (U32 i = 0; i < m_triangleCount; ++i)
	{
		auto &it = m_navMeshTriangles[i];

		if (it.m_count == 0)
		{
			result = false;
			break;
		}
	}*/

	return result;
}

U32 NavMesh::testRayAgainstVertices(const Ray &ray)
{
	U32 result = 0xffffffff;
#if _VX_EDITOR
	for (U32 i = 0; i < m_vertexCount; ++i)
	{
		auto &it = m_vertexBounds[i];
		if (it.intersects(ray, nullptr, nullptr))
		{
			result = i;
			break;
		}
	}
#else
	VX_UNREFERENCED_PARAMETER(ray);
#endif
	return result;
}

bool NavMesh::isCCW(const vx::float3 &p0, const vx::float3 &p1, const vx::float3 &p2) const
{
	auto det = p0.x * p1.y * p2.z - p0.x * p2.y * p1.z - p1.x * p0.y * p2.z + p1.x * p2.y * p0.z + p2.x * p0.y * p1.z - p2.x * p1.y * p0.z;

	return det > 0.0f;
}

const vx::float3* NavMesh::getVertices() const 
{
#if _VX_EDITOR
	return m_vertices.data(); 
#else
	return m_vertices.get();
#endif
}

const TriangleIndices* NavMesh::getTriangleIndices() const
{ 
#if _VX_EDITOR
	return m_triangleIndices.data();
#else
	return m_triangleIndices.get();
#endif
}

const NavMeshTriangle* NavMesh::getNavMeshTriangles() const
{
	return m_navMeshTriangles.get();
}