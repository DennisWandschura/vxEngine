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

template<typename T>
class Pool;

struct Event;
struct EntityActor;
class EntityAspect;
class PhysicsAspect;
class EventManager;

#include "EventListener.h"
#include "NavGraph.h"
#include "InfluenceMap.h"
#include "ActionManager.h"
#include <vxLib/Allocator/StackAllocator.h>
#include <random>
#include "SquadManager.h"

class ActorAspect : public EventListener
{
	const Pool<Component::Actor>* m_pActorPool{ nullptr };
	//const Pool<Component::Physics>* m_pPhysicsPool{ nullptr };
	const Pool<EntityActor>* m_pEntityPool{ nullptr };
	ai::SquadManager m_squadManager;
	ActionManager m_actionManager;
	InfluenceMap m_influenceMap;
	NavGraph m_navGraph;
	const PhysicsAspect &m_physicsAspect;
	std::mt19937_64 m_gen;
	vx::StackAllocator m_allocator;
	vx::StackAllocator m_allocatorScratch;

	void handleRequestPath(Component::Actor* pActor);

	///////////////////
	// Event functions
	///////////////////

	void handleAIEvent(const Event &evt);
	void handleFileEvent(const Event &evt);
	void handleIngameEvent(const Event &evt);

	///////////////////

public:
	explicit ActorAspect(const PhysicsAspect &physicsAspect);

	///////////////////

	void initialize(const EntityAspect &entityAspect, EventManager &evtManager, vx::StackAllocator* pAllocator);
	void shutdown();

	///////////////////

	///////////////////
	// update functions
	///////////////////

	void update(f32 dt);

	///////////////////

	///////////////////

	void handleEvent(const Event &evt);

	///////////////////
};