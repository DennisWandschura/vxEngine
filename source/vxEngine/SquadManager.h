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