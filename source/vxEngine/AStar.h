#pragma once

struct NavConnection;
class NavGraph;
struct NavNode;
class PhysicsAspect;

namespace vx
{
	class StackAllocator;

	template<typename T>
	class array;
}

#include <vector>
#include <vxLib/math/Vector.h>

typedef F32(*HeuristicFp)(const NavNode &fromNode, const NavNode &goalNode);

extern F32 heuristicDistance(const NavNode &fromNode, const NavNode &goalNode);
extern F32 heuristicDistance2(const NavNode &fromNode, const NavNode &goalNode);

// returns list of node ids in reverse order (start node is at the back and goal at the front)
extern U8 pathfindAStar(const NavGraph &graph, U16 start, U16 goal, HeuristicFp fp, vx::StackAllocator* pAllocatorScratch, vx::array<vx::float3>* out, const PhysicsAspect* pPhysicsAspect = nullptr);