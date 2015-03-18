#pragma once

struct NavConnection;
struct NavNode;
class NavMesh;
namespace vx
{
	class StackAllocator;
}

#include <vxLib/math/Vector.h>
#include <memory>
#include "Grid.h"

class NavGraph
{
	static const U8 s_cellSize{3};

	NavConnection* m_connections{nullptr};
	NavNode* m_nodes{nullptr};
	U32 m_connectionCount{0};
	U32 m_nodeCount{0};

public:
	NavGraph();
	~NavGraph();

	void initialize(const NavMesh &navMesh, vx::StackAllocator* pAllocator, vx::StackAllocator* pAllocatorScratch);
	void shutdown(vx::StackAllocator* pAllocator);

	const NavConnection* getConnections() const { return m_connections; }
	const NavNode* getNodes() const { return m_nodes; }
	const NavNode& getNode(U32 i) const;

	U32 getConnectionCount() const { return m_connectionCount; }
	U32 getNodeCount() const { return m_nodeCount; }

	U32 getClosestNode(const vx::float3 &position) const;
	U32 getFarestNode(const vx::float3 &position) const;
};