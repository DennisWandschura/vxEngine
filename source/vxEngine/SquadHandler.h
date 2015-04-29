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

		U16 m_actors[4];
		U8 m_updateMask{ 0 };
		U8 m_size{ 0 };
		U16 m_filterMask;
		U32 m_currentTargetNodes[4];

		void updateActor(U8 index, vx::StackAllocator* pAllocatorScratch);
		void updateActor(U8 index, const InfluenceCell* pCells, U32 cellIndex, const U16* pNavNodeIndices, vx::StackAllocator* pAllocatorScratch);

		void handleRequestPath(Component::Actor* p);

	public:
		explicit SquadHandler(U16 filterMask);

		static void initializeStatics(const NavGraph* pNavGrap, const Pool<Component::Actor>* pActorPool, const Pool<Component::Physics>* pPhysicsPool, const Pool<EntityActor>* pEntityPool, const InfluenceMap* pInfluenceMap);

		void update(vx::StackAllocator* pAllocatorScratch);

		void handleAIEvent(const Event &evt);

		U8 addActor(U16 actorIndex);

		U16 getFilter() const { return m_filterMask; }
		U8 size() const { return m_size; }
		U8 hasSpace() const { return m_size < 4; }
	};
}