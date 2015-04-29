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

	void update(F32 dt);

	///////////////////

	///////////////////

	void handleEvent(const Event &evt);

	///////////////////
};