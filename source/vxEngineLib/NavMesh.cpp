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
#include <vxLib/string.h>

NavMesh::NavMesh()
	:m_navMeshTriangles(),
	m_vertices(),
	m_triangleIndices(),
	m_bounds(),
	m_vertexCount(0),
	m_triangleCount(0),
	m_vertexBounds()
{
}

NavMesh::NavMesh(NavMesh &&rhs)
	: m_navMeshTriangles(std::move(rhs.m_navMeshTriangles)),
	m_vertices(std::move(rhs.m_vertices)),
	m_triangleIndices(std::move(rhs.m_triangleIndices)),
	m_bounds(std::move(rhs.m_bounds)),
	m_vertexCount(rhs.m_vertexCount),
	m_triangleCount(rhs.m_triangleCount),
	m_vertexBounds(std::move(rhs.m_vertexBounds))
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

		std::swap(m_vertexBounds, rhs.m_vertexBounds);

		std::swap(m_bounds, rhs.m_bounds);
		std::swap(m_vertexCount, rhs.m_vertexCount);
		std::swap(m_triangleCount, rhs.m_triangleCount);
	}

	return *this;
}

void NavMesh::reset()
{
	m_navMeshTriangles.reset();

	m_vertices.clear();
	m_triangleIndices.clear();

	m_bounds = AABB();
	m_vertexCount = 0;
	m_triangleCount = 0;
}

void NavMesh::copy(NavMesh* dst) const
{
	dst->m_vertices.clear();
	dst->m_vertices = m_vertices;

	dst->m_triangleIndices.clear();
	dst->m_triangleIndices = m_triangleIndices;

	dst->m_vertexBounds.clear();
	dst->m_vertexBounds = m_vertexBounds;

	if (m_triangleCount != 0)
	{
		copyUniquePtr(&dst->m_navMeshTriangles, m_navMeshTriangles, m_triangleCount);
	}

	dst->m_bounds = m_bounds;
	dst->m_vertexCount = m_vertexCount;
	dst->m_triangleCount = m_triangleCount;
}

void NavMesh::swap(NavMesh &other)
{
	std::swap(other.m_vertexBounds, m_vertexBounds);

	std::swap(other.m_vertices, m_vertices);
	std::swap(other.m_triangleIndices, m_triangleIndices);

	std::swap(other.m_navMeshTriangles, m_navMeshTriangles);
	std::swap(other.m_bounds, m_bounds);
	std::swap(other.m_vertexCount, m_vertexCount);
	std::swap(other.m_triangleCount, m_triangleCount);
}

void NavMesh::saveToFile(vx::File *file) const
{
	file->write(m_vertexCount);
	file->write(m_triangleCount);
	file->write(m_bounds);

	file->write(m_vertices.data(), m_vertexCount);
	file->write(m_triangleIndices.data(), m_triangleCount * 3);
}

const u8* NavMesh::load(const u8 *ptr)
{
	ptr = vx::read(m_vertexCount, ptr);
	ptr = vx::read(m_triangleCount, ptr);
	ptr = vx::read(m_bounds, ptr);

	auto indexCount = m_triangleCount * 3;

	m_vertices = std::vector<vx::float3>(m_vertexCount);
	m_triangleIndices = std::vector<u16>(indexCount);

	ptr = vx::read(m_vertices.data(), ptr, m_vertexCount);
	ptr = vx::read(m_triangleIndices.data(), ptr, indexCount);

	m_vertexBounds.reserve(m_vertexCount);
	for (auto &it : m_vertices)
	{
		AABB bounds;
		bounds.min = it - vx::float3(0.05f);
		bounds.max = it + vx::float3(0.05f);

		m_vertexBounds.push_back(bounds);
}

	m_navMeshTriangles = createNavMeshTriangles();

	return ptr;
}
void NavMesh::addVertex(const vx::float3 &vertex)
{
	m_vertices.push_back(vertex);
	++m_vertexCount;

	m_bounds = AABB::merge(m_bounds, vertex);

	AABB bounds;
	bounds.min = vertex - vx::float3(0.05f);
	bounds.max = vertex + vx::float3(0.05f);

	m_vertexBounds.push_back(bounds);
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
	m_bounds = AABB();
	for (auto &it : m_vertices)
	{
		m_bounds = AABB::merge(m_bounds, it);
	}
}

bool NavMesh::removeVertex(const vx::float3 &position)
{
	bool found = false;
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
		removeVertex(index);
	}

	return found;
}

void NavMesh::findAndEraseVertexFromIndices(u16 vertexIndex)
{
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
}

void NavMesh::fixVertexIndicesAfterErasedVertex(u16 erasedVertexIndex)
{
	for (auto &it : m_triangleIndices)
	{
		if (it >= erasedVertexIndex)
		{
			--it;
		}
	}
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
	m_vertices.erase(m_vertices.begin() + index);
	m_vertexBounds.erase(m_vertexBounds.begin() + index);
	--m_vertexCount;

	buildBounds();

	findAndEraseVertexFromIndices(index);

	fixVertexIndicesAfterErasedVertex(index);

	m_navMeshTriangles = createNavMeshTriangles();
}

void NavMesh::addTriangle(const u32(&selectedIndices)[3])
{
	auto v0 = m_vertices[selectedIndices[0]];
	auto v1 = m_vertices[selectedIndices[1]];
	auto v2 = m_vertices[selectedIndices[2]];

	printf("%u %u %u\n", selectedIndices[0], selectedIndices[1], selectedIndices[2]);
	printf("%f %f %f\n", v0.x, v0.y, v0.z);
	printf("%f %f %f\n", v1.x, v1.y, v1.z);
	printf("%f %f %f\n", v2.x, v2.y, v2.z);

	if (isCCW(v0, v1, v2))
	{
		printf("derpoo2\n");
		m_triangleIndices.push_back(selectedIndices[0]);
		m_triangleIndices.push_back(selectedIndices[1]);
		m_triangleIndices.push_back(selectedIndices[2]);

		++m_triangleCount;

		printf("derpoo3\n");
		m_navMeshTriangles = createNavMeshTriangles();
	}
	else
	{
		printf("not ccw !\n");
	}
}

void NavMesh::removeTriangle()
{
	m_triangleIndices.pop_back();
	m_triangleIndices.pop_back();
	m_triangleIndices.pop_back();

	--m_triangleCount;

	m_navMeshTriangles = createNavMeshTriangles();
	buildBounds();
}

void NavMesh::setVertexPosition(u32 i, const vx::float3 &position)
{
	m_vertices[i] = position;
	m_vertexBounds[i].min = position - vx::float3(0.05f);
	m_vertexBounds[i].max = position + vx::float3(0.05f);

	buildBounds();

	m_navMeshTriangles = createNavMeshTriangles();
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
	for (u32 i = 0; i < m_vertexCount; ++i)
	{
		auto &it = m_vertexBounds[i];
		if (it.intersects(ray, nullptr, nullptr))
		{
			result = i;
			break;
		}
}
	return result;
}

bool NavMesh::getIndex(const vx::float3 &position, u32* index) const
{
	bool result = false;

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

	return result;
}

bool NavMesh::isCCW(const vx::float3 &p0, const vx::float3 &p1, const vx::float3 &p2) const
{
	const vx::float3 n = { 1, 1, 1 };

	auto normal = vx::cross(p1 - p0, p2 - p0);
	auto det = vx::dot3(normal, n);
	printf("%f\n",det);

	return (det > 0.0f);
}

const vx::float3* NavMesh::getVertices() const
{
	return m_vertices.data();
}

const u16* NavMesh::getTriangleIndices() const
{
	return m_triangleIndices.data();
}

const NavMeshTriangle* NavMesh::getNavMeshTriangles() const
{
	return m_navMeshTriangles.get();
}