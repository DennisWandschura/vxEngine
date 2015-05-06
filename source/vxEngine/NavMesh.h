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
#pragma once

class File;
struct NavMeshTriangle;

namespace YAML
{
	class Node;
}

#include <vxLib/math/Vector.h>
#if _VX_EDITOR
#include <vector>
#endif
#include <memory>
#include "AABB.h"

struct TriangleIndices
{
	U16 vertexIndex[3];
};

class NavMesh
{
	std::unique_ptr<NavMeshTriangle[]> m_navMeshTriangles;
#if _VX_EDITOR
	std::vector<vx::float3> m_vertices;
	std::vector<TriangleIndices> m_triangleIndices;
	std::vector<AABB> m_vertexBounds;
#else
	std::unique_ptr<vx::float3[]> m_vertices;
	std::unique_ptr<TriangleIndices[]> m_triangleIndices;
#endif
	AABB m_bounds;
	U32 m_vertexCount;
	U32 m_triangleCount;

	bool isCCW(const vx::float3 &p0, const vx::float3 &p1, const vx::float3 &p2) const;

	void findAndEraseVertexFromIndices(U16 vertexIndex);
	void fixVertexIndicesAfterErasedVertex(U16 erasedVertexIndex);

	std::unique_ptr<NavMeshTriangle[]> createNavMeshTriangles();

public:
	/////////////// constructors
	NavMesh();
	NavMesh(const NavMesh &rhs) = delete;
	NavMesh(NavMesh &&rhs);
	~NavMesh();
	/////////////// constructors

	/////////////// operators
	NavMesh& operator=(const NavMesh &rhs) = delete;
	NavMesh& operator=(NavMesh &&rhs);
	/////////////// operators

	void copyTo(NavMesh* other) const;

	/////////////// loading
	//void loadFromYAML(const YAML::Node &node);
	const U8* load(const U8 *ptr);
	/////////////// loading

	/////////////// saving
	void saveToFile(File *file) const;
	//void saveToYAML(YAML::Node &node) const;
	/////////////// saving

	/////////////// getters
	const vx::float3* getVertices() const;
	const TriangleIndices* getTriangleIndices() const;
	const NavMeshTriangle* getNavMeshTriangles() const;

	U32 getVertexCount() const { return m_vertexCount; }
	U32 getTriangleCount() const { return m_triangleCount; }

	const AABB& getBounds() const { return m_bounds; }
	/////////////// getters

	void addVertex(const vx::float3 &vertex);
	void deleteVertex(U32 index);
	void addTriangle(const U32(&indices)[3]);
	void setVertexPosition(U32 i, const vx::float3 &position);

	bool isValid() const;

	// returns U32 max on failure
	U32 testRayAgainstVertices(const Ray &ray);
};