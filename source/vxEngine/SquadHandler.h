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

namespace Component
{
	struct Actor;
	struct Physics;
}

namespace vx
{
	class StackAllocator;
}

template<typename T>
class Pool;

class NavGraph;
class InfluenceMap;
struct Event;
struct EntityActor;
struct InfluenceCell;

#include <vxLib/math/Vector.h>
#include <random>

namespace ai
{
	class SquadHandler
	{
		//static const Pool<Component::Physics>* s_pPhysicsPool;
		static const Pool<Component::Actor>* s_pActorPool;
		static const Pool<EntityActor>* s_pEntityPool;
		static const InfluenceMap* s_pInfluenceMap;
		static const NavGraph* s_pNavGraph;
		static std::mt19937_64 s_gen;

		u16 m_actors[4];
		u8 m_updateMask{ 0 };
		u8 m_size{ 0 };
		u16 m_filterMask;
		u32 m_currentTargetNodes[4];

		void updateActor(u8 index, vx::StackAllocator* pAllocatorScratch);
		void updateActor(u8 index, const InfluenceCell* pCells, u32 cellIndex, const u16* pNavNodeIndices, vx::StackAllocator* pAllocatorScratch);

		void handleRequestPath(Component::Actor* p);

	public:
		explicit SquadHandler(u16 filterMask);

		static void initializeStatics(const NavGraph* pNavGrap, const Pool<Component::Actor>* pActorPool, const Pool<Component::Physics>* pPhysicsPool, const Pool<EntityActor>* pEntityPool, const InfluenceMap* pInfluenceMap);

		void update(vx::StackAllocator* pAllocatorScratch);

		void handleAIEvent(const Event &evt);

		u8 addActor(u16 actorIndex);

		u16 getFilter() const { return m_filterMask; }
		u8 size() const { return m_size; }
		u8 hasSpace() const { return m_size < 4; }
	};
}