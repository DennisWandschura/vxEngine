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

class NavMesh;
struct NavMeshNode;
struct NavMeshTriangle;

namespace vx
{
	template<typename K, typename T, typename C>
	class sorted_vector;
}

#include <vxLib/math/Vector.h>
#include <memory>

struct NavMeshNode
{
	vx::float3 position;
	u32 connectionIndex;
	u32 connectionCount;
};

class NavMeshGraph
{
	struct Node;
	struct Connection;
	struct BuildNode;
	struct CompareFloat3;

	std::unique_ptr<NavMeshNode[]> m_nodes;
	std::unique_ptr<u16[]> m_connections;
	const NavMesh* m_pNavMesh;
	u32 m_nodeCount;
	u32 m_connectionCount;

	void insertTriangles(const NavMeshTriangle* navMeshTriangles, u32 triangleCount, vx::sorted_vector<vx::float3, BuildNode, CompareFloat3>* sortedBuildNodes);
	void insertOrMergeBuildNode(const BuildNode &node, vx::sorted_vector<vx::float3, BuildNode, CompareFloat3>* sortedBuildNodes);
	void fixConnectionIndices(u32 nodeCount, vx::sorted_vector<vx::float3, BuildNode, CompareFloat3>* sortedBuildNodes, u32* connectionCount);
	std::unique_ptr<NavMeshNode[]> buildNodes(const NavMesh &navMesh, u32* finalNodeCount);

public:
	NavMeshGraph();
	~NavMeshGraph();

	void initialize(const NavMesh &navMesh);

	const NavMeshNode* getNodes() const;
	u32 getNodeCount() const;

	u32 getConnectionCount() const;
	const u16* getConnections() const;

	u32 getClosestNodeInex(const vx::float3 &position) const;
};