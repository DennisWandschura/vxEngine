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
#include <vxEngineLib/NavMesh.h>
#include <vxEngineLib/NavMeshTriangle.h>
#include <vxLib/File/File.h>
#include <vxLib/algorithm.h>
#include <vxEngineLib/copy.h>
#include <vxEngineLib/memcpy.h>

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

void NavMesh::copy(NavMesh* dst) const
{
#if _VX_EDITOR
	dst->m_vertices.clear();
	dst->m_vertices = m_vertices;

	dst->m_triangleIndices.clear();
	dst->m_triangleIndices = m_triangleIndices;

	dst->m_vertexBounds.clear();
	dst->m_vertexBounds = m_vertexBounds;
#else
	copyUniquePtr(&dst->m_vertices, m_vertices, m_vertexCount);

	auto indexCount = m_triangleCount * 3;
	copyUniquePtr(&dst->m_triangleIndices, m_triangleIndices, indexCount);
#endif

	copyUniquePtr(&dst->m_navMeshTriangles, m_navMeshTriangles, m_triangleCount);

	dst->m_bounds = m_bounds;
	dst->m_vertexCount = m_vertexCount;
	dst->m_triangleCount = m_triangleCount;
}

void NavMesh::saveToFile(vx::File *file) const
{
	file->write(m_vertexCount);
	file->write(m_triangleCount);
	file->write(m_bounds);

#if _VX_EDITOR
	file->write(m_vertices.data(), m_vertexCount);
	file->write(m_triangleIndices.data(), m_triangleCount * 3);
#else
	file->write(m_vertices.get(), m_vertexCount);
	file->write(m_triangleIndices.get(), m_triangleCount * 3);
#endif
}

const u8* NavMesh::load(const u8 *ptr)
{
	ptr = vx::read(m_vertexCount, ptr);
	ptr = vx::read(m_triangleCount, ptr);
	ptr = vx::read(m_bounds, ptr);

	#warning Please use a custom allocator !

	auto indexCount = m_triangleCount * 3;

#if _VX_EDITOR
	m_vertices = std::vector<vx::float3>(m_vertexCount);
	m_triangleIndices = std::vector<u16>(indexCount);

	ptr = vx::read(m_vertices.data(), ptr, m_vertexCount);
	ptr = vx::read(m_triangleIndices.data(), ptr, indexCount);

	m_vertexBounds.reserve(m_vertexCount);
	for (auto &it : m_vertices)
	{
		AABB bounds;
		bounds.min = it - vx::float3(0.1f);
		bounds.max = it + vx::float3(0.1f);

		m_vertexBounds.push_back(bounds);
	}
#else
	m_vertices = vx::make_unique<vx::float3[]>(m_vertexCount);
	m_triangleIndices = vx::make_unique<u16[]>(indexCount);

	ptr = vx::read(m_vertices.get(), ptr, m_vertexCount);
	ptr = vx::read(m_triangleIndices.get(), ptr, indexCount);
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

bool NavMesh::contains(const vx::float3 &p) const
{
	bool result = false;

	if (m_bounds.contains(p))
	{
		for (u32 i = 0; i < m_triangleCount; ++i)
		{
			if (m_navMeshTriangles[i].m_triangle.contains(p))
			{
				result = true;
				break;
			}
		}
	}

	return result;
}

void NavMesh::buildBounds()
{
#if _VX_EDITOR
	m_bounds = AABB();
	for (auto &it : m_vertices)
	{
		m_bounds = AABB::merge(m_bounds, it);
	}
#endif
}

bool NavMesh::removeVertex(const vx::float3 &position)
{
	bool found = false;
#if _VX_EDITOR
	u32 index = 0;
	for (auto &it : m_vertexBounds)
	{
		if (it.contains(position))
		{
			found = true;
			break;
		}

		++index;
	}

	if (found)
	{
		m_vertexBounds.erase(m_vertexBounds.begin() + index);
		m_vertices.erase(m_vertices.begin() + index);
		--m_vertexCount;
	}

	buildBounds();
#endif
	return found;
}

void NavMesh::findAndEraseVertexFromIndices(u16 vertexIndex)
{
#if _VX_EDITOR
	auto it = m_triangleIndices.begin();
	while (it != m_triangleIndices.end())
	{
		auto iter = m_triangleIndices.begin();
		while (iter != m_triangleIndices.end())
		{
			if (*iter == vertexIndex || 
				*(iter + 1) == vertexIndex || 
				*(iter+2) == vertexIndex)
			{
				break;
			}

			iter += 3;
		}
		it = iter;

		if (it != m_triangleIndices.end())
		{
			it = m_triangleIndices.erase(it, it + 3);
			--m_triangleCount;
		}
	}
#else
	VX_UNREFERENCED_PARAMETER(vertexIndex);
#endif
}

void NavMesh::fixVertexIndicesAfterErasedVertex(u16 erasedVertexIndex)
{
#if _VX_EDITOR
	for (auto &it : m_triangleIndices)
	{
		if (it >= erasedVertexIndex)
		{
			--it;
		}
	}
#else
	VX_UNREFERENCED_PARAMETER(erasedVertexIndex);
#endif
}

std::unique_ptr<NavMeshTriangle[]> NavMesh::createNavMeshTriangles()
{
	auto ptr = vx::make_unique<NavMeshTriangle[]>(m_triangleCount);

	auto indexCount = m_triangleCount * 3;

	u32 index = 0;
	for (u32 i = 0; i < m_triangleCount; ++i)
	{
		ptr[i].m_triangle[0] = m_vertices[m_triangleIndices[index++]];
		ptr[i].m_triangle[1] = m_vertices[m_triangleIndices[index++]];
		ptr[i].m_triangle[2] = m_vertices[m_triangleIndices[index++]];
		ptr[i].m_count = 0;
	}

	VX_ASSERT(indexCount == index);

	for (u32 j = 0; j < m_triangleCount; ++j)
	{
		auto &it = ptr[j];

		for (u32 i = 0; i < m_triangleCount; ++i)
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

void NavMesh::removeVertex(u32 index)
{
#if _VX_EDITOR
	m_vertices.erase(m_vertices.begin() + index);
	m_vertexBounds.erase(m_vertexBounds.begin() + index);
	--m_vertexCount;

	buildBounds();
	
	findAndEraseVertexFromIndices(index);

	fixVertexIndicesAfterErasedVertex(index);

	m_navMeshTriangles = createNavMeshTriangles();
#else
	VX_UNREFERENCED_PARAMETER(index);
#endif
}

void NavMesh::addTriangle(const u32(&selectedIndices)[3])
{
#if _VX_EDITOR
	auto v0 = m_vertices[selectedIndices[0]];
	auto v1 = m_vertices[selectedIndices[1]];
	auto v2 = m_vertices[selectedIndices[2]];

	if (isCCW(v0, v1, v2))
	{
		m_triangleIndices.push_back(selectedIndices[0]);
		m_triangleIndices.push_back(selectedIndices[1]);
		m_triangleIndices.push_back(selectedIndices[2]);

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

void NavMesh::removeTriangle()
{
#if _VX_EDITOR
	m_triangleIndices.pop_back();
	m_triangleIndices.pop_back();
	m_triangleIndices.pop_back();

	--m_triangleCount;

	m_navMeshTriangles = createNavMeshTriangles();
	buildBounds();
#endif
}

void NavMesh::setVertexPosition(u32 i, const vx::float3 &position)
{
#if _VX_EDITOR
	m_vertices[i] = position;
	m_vertexBounds[i].min = position - vx::float3(0.1f);
	m_vertexBounds[i].max = position + vx::float3(0.1f);

	buildBounds();

	m_navMeshTriangles = createNavMeshTriangles();
#else
	VX_UNREFERENCED_PARAMETER(i);
	VX_UNREFERENCED_PARAMETER(position);
#endif
}

bool NavMesh::isValid() const
{
	bool result = true;
	/*for (u32 i = 0; i < m_triangleCount; ++i)
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

u32 NavMesh::testRayAgainstVertices(const Ray &ray)
{
	u32 result = 0xffffffff;
#if _VX_EDITOR
	for (u32 i = 0; i < m_vertexCount; ++i)
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

bool NavMesh::getIndex(const vx::float3 &position, u32* index) const
{
	bool result = false;

#if _VX_EDITOR
	for (u32 i = 0; i < m_vertexCount; ++i)
	{
		auto &it = m_vertexBounds[i];
		if (it.contains(position))
		{
			result = true;
			*index = i;
			break;
		}
	}
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

const u16* NavMesh::getTriangleIndices() const
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