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

struct NavConnection;
class NavGraph;
struct NavNode;
class PhysicsAspect;
class NavMeshGraph;

namespace vx
{
	class StackAllocator;

	template<typename T>
	class array;
}

#include <vector>
#include <vxLib/math/Vector.h>

typedef f32(*HeuristicFp)(const vx::float3 &fromNode, const vx::float3 &goalNode);

extern f32 heuristicDistance(const vx::float3 &fromNode, const vx::float3 &goalNode);
extern f32 heuristicDistance2(const vx::float3 &fromNode, const vx::float3 &goalNode);

// returns list of node ids in reverse order (start node is at the back and goal at the front)
extern u8 pathfindAStar(const NavGraph &graph, u16 start, u16 goal, HeuristicFp fp, vx::StackAllocator* pAllocatorScratch, vx::array<vx::float3>* out, const PhysicsAspect* pPhysicsAspect = nullptr);
extern bool pathfindAStar(const NavMeshGraph &graph, u16 start, u16 goal, HeuristicFp fp, vx::StackAllocator* pAllocatorScratch, vx::array<vx::float3>* out);
