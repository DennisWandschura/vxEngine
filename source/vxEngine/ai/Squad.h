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

struct Waypoint;
struct EntityActor;
class InfluenceMap;
class NavMeshGraph;

namespace Component
{
	struct Actor;
}

#include <vxLib/types.h>
#include <vector>
#include <vxLib/Allocator/StackAllocator.h>
#include "../PseudoRandom.h"
#include <random>

namespace ai
{
	class Squad
	{
		static InfluenceMap* s_influenceMap;
		static NavMeshGraph* s_navmeshGraph;

		struct Data
		{
			EntityActor* m_entity;
			Component::Actor* m_actorComponent;
			std::vector<Waypoint> m_waypoints;
			u16 m_cells[2];
			u8 m_cellCount;
		};

		std::vector<Data> m_entities;
		PseudoRandom m_pseudoRandom;
		vx::StackAllocator m_scratchAllocator;
		f32 m_avgCoverageArea;
		std::vector<u32> m_availableCells;
		std::mt19937_64 m_gen;

	public:
		Squad();
		~Squad();

		void initialize(vx::StackAllocator* allocator);

		bool addEntity(EntityActor* entity, Component::Actor* actorComponent);

		void createPath(Component::Actor* componentActor);

		static void provide(InfluenceMap* influenceMap, NavMeshGraph* graph);
		void updateAfterProvide();
	};
}