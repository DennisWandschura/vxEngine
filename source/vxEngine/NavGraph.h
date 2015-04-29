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
#if _VX_EDITOR
#include <vector>
#endif

class NavGraph
{
	static const U8 s_cellSize{3};

#if _VX_EDITOR
	std::vector<NavConnection> m_connections;
	std::vector<NavNode> m_nodes;
#else
	NavConnection* m_connections;
	NavNode* m_nodes;
#endif
	U32 m_connectionCount;
	U32 m_nodeCount;

public:
	NavGraph();
	~NavGraph();

	void initialize(const NavMesh &navMesh, vx::StackAllocator* pAllocator, vx::StackAllocator* pAllocatorScratch);
	void shutdown(vx::StackAllocator* pAllocator);

#if _VX_EDITOR
	const NavConnection* getConnections() const { return m_connections.data(); }
	const NavNode* getNodes() const { return m_nodes.data(); }
#else
	const NavConnection* getConnections() const { return m_connections; }
	const NavNode* getNodes() const { return m_nodes; }
#endif
	const NavNode& getNode(U32 i) const;

	U32 getConnectionCount() const { return m_connectionCount; }
	U32 getNodeCount() const { return m_nodeCount; }

	U32 getClosestNode(const vx::float3 &position) const;
	U32 getFarestNode(const vx::float3 &position) const;
};