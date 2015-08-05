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
	static const u8 s_cellSize{3};

#if _VX_EDITOR
	std::vector<NavConnection> m_connections;
	std::vector<NavNode> m_nodes;
#else
	NavConnection* m_connections;
	NavNode* m_nodes;
#endif
	u32 m_connectionCount;
	u32 m_nodeCount;

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
	const NavNode& getNode(u32 i) const;

	u32 getConnectionCount() const { return m_connectionCount; }
	u32 getNodeCount() const { return m_nodeCount; }

	u32 getClosestNode(const vx::float3 &position) const;
	u32 getFarestNode(const vx::float3 &position) const;
};