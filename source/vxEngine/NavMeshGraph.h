#pragma once

class NavMesh;
struct NavMeshNode;
struct NavMeshConnection;

#include <vxLib/math/Vector.h>
#include <memory>

struct NavMeshNode
{
	vx::float3 position;
	U32 connectionIndex;
};

class NavMeshGraph
{
	struct Node;
	struct Connection;

	std::unique_ptr<NavMeshNode[]> m_nodes;
	const NavMesh* m_pNavMesh;
	U32 m_nodeCount;

	std::unique_ptr<NavMeshNode[]> buildNodes(const NavMesh &navMesh, U32* finalNodeCount);

public:
	NavMeshGraph();
	~NavMeshGraph();

	void initialize(const NavMesh &navMesh);

	const NavMeshNode* getNodes() const;
	U32 getNodeCount() const;
};