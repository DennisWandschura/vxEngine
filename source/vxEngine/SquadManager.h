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

#include "EventListener.h"
#include <vector>
#include <vxLib/types.h>

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

struct EntityActor;
class NavGraph;
class InfluenceMap;

namespace ai
{
	class SquadHandler;

	class SquadManager : public EventListener
	{
		static U16 s_squadFilterMask;

		std::vector<SquadHandler> m_squadHandlers;
		const NavGraph* m_pNavGraph{ nullptr };
		const InfluenceMap* m_pInfluenceMap{ nullptr };

		void handleAIEvent(const Event &evt);

	public:
		SquadManager();
		~SquadManager();

		void initialize(const Pool<Component::Actor>* pActorPool, const NavGraph* pNavGraph, const InfluenceMap* pInfluenceMap, const Pool<Component::Physics>* pPhysicsPool, const Pool<EntityActor>* pEntityPool);

		void update(vx::StackAllocator* pAllocatorScratch);

		void handleEvent(const Event &evt) override;

		U32 createSquadHandler();

		// returns index of squad handler
		U32 addActor(U16 actorIndex);

		const SquadHandler& getSquadHandler(U32 i) const;
		SquadHandler& getSquadHandler(U32 i);
	};
}