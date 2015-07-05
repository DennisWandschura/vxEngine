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
	template<typename T>
	class Pool;

	struct Event;
}

struct EntityActor;
class EntityAspect;
class PhysicsAspect;
class EventManager;
class Scene;

#include <vxEngineLib/EventListener.h>
#include <vxEngineLib/InfluenceMap.h>
#include "ActionManager.h"
#include <vxLib/Allocator/StackAllocator.h>
#include <vxEngineLib/NavMeshGraph.h>
#include "ai/Squad.h"

class ActorAspect : public vx::EventListener
{
	ai::Squad m_squad;
	const vx::Pool<Component::Actor>* m_pActorPool{ nullptr };
	const vx::Pool<EntityActor>* m_pEntityPool{ nullptr };
	ActionManager m_actionManager;
	InfluenceMap m_influenceMap;
	NavMeshGraph m_navmeshGraph;
	const PhysicsAspect &m_physicsAspect;
	vx::StackAllocator m_allocator;
	vx::StackAllocator m_allocatorScratch;

	///////////////////
	// Event functions
	///////////////////

	void handleFileEvent(const vx::Event &evt);
	void handleIngameEvent(const vx::Event &evt);

	///////////////////

	void createInfluenceMap(const Scene* scene);

public:
	explicit ActorAspect(const PhysicsAspect &physicsAspect);

	void initialize(const EntityAspect &entityAspect, vx::StackAllocator* pAllocator);
	void shutdown();

	void update();

	void handleEvent(const vx::Event &evt);
};