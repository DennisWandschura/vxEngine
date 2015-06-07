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

namespace vx
{
	class File;
}
struct NavMeshTriangle;

#include <vxLib/math/Vector.h>
#if _VX_EDITOR
#include <vector>
#endif
#include <vxLib/memory.h>
#include <vxEngineLib/AABB.h>

class NavMesh
{
	std::unique_ptr<NavMeshTriangle[]> m_navMeshTriangles;
#if _VX_EDITOR
	std::vector<vx::float3> m_vertices;
	std::vector<u16> m_triangleIndices;
	std::vector<AABB> m_vertexBounds;
#else
	std::unique_ptr<vx::float3[]> m_vertices;
	std::unique_ptr<u16[]> m_triangleIndices;
#endif
	AABB m_bounds;
	u32 m_vertexCount;
	u32 m_triangleCount;

	bool isCCW(const vx::float3 &p0, const vx::float3 &p1, const vx::float3 &p2) const;

	void findAndEraseVertexFromIndices(u16 vertexIndex);
	void fixVertexIndicesAfterErasedVertex(u16 erasedVertexIndex);

	std::unique_ptr<NavMeshTriangle[]> createNavMeshTriangles();

	void buildBounds();

public:
	/////////////// constructors
	NavMesh();
	NavMesh(const NavMesh &) = delete;
	NavMesh(NavMesh &&rhs);
	~NavMesh();
	/////////////// constructors

	/////////////// operators
	NavMesh& operator=(const NavMesh &rhs) = delete;
	NavMesh& operator=(NavMesh &&rhs);
	/////////////// operators

	void copy(NavMesh* dst) const;

	/////////////// loading
	//void loadFromYAML(const YAML::Node &node);
	const u8* load(const u8 *ptr);
	/////////////// loading

	/////////////// saving
	void saveToFile(vx::File *file) const;
	//void saveToYAML(YAML::Node &node) const;
	/////////////// saving

	/////////////// getters
	const vx::float3* getVertices() const;
	const u16* getTriangleIndices() const;
	const NavMeshTriangle* getNavMeshTriangles() const;

	u32 getVertexCount() const { return m_vertexCount; }
	u32 getTriangleCount() const { return m_triangleCount; }

	const AABB& getBounds() const { return m_bounds; }
	/////////////// getters

	void addVertex(const vx::float3 &vertex);
	bool removeVertex(const vx::float3 &vertex);
	void removeVertex(u32 index);
	void addTriangle(const u32(&indices)[3]);
	void removeTriangle();
	void setVertexPosition(u32 i, const vx::float3 &position);

	bool isValid() const;

	// returns u32 max on failure
	u32 testRayAgainstVertices(const Ray &ray);
	bool getIndex(const vx::float3 &position, u32* index) const;
};