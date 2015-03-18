#include "SquadManager.h"
#include "Event.h"
#include "SquadHandler.h"
#include "EventTypes.h"

namespace ai
{
	U16 SquadManager::s_squadFilterMask{ 0 };

	SquadManager::SquadManager()
		:m_squadHandlers()
	{

	}

	SquadManager::~SquadManager()
	{

	}

	void SquadManager::initialize(const Pool<Component::Actor>* pActorPool, const NavGraph* pNavGraph, const InfluenceMap* pInfluenceMap, const Pool<Component::Physics>* pPhysicsPool, const Pool<Entity>* pEntityPool)
	{
		m_pNavGraph = pNavGraph;
		m_pInfluenceMap = pInfluenceMap;

		SquadHandler::initializeStatics(pNavGraph, pActorPool, pPhysicsPool, pEntityPool, pInfluenceMap);
	}

	void SquadManager::update(vx::StackAllocator* pAllocatorScratch)
	{
		for (auto &it : m_squadHandlers)
		{
			it.update(pAllocatorScratch);
		}
	}

	void SquadManager::handleEvent(const Event &evt)
	{
		if (evt.type == EventType::AI_Event)
		{
			handleAIEvent(evt);
		}
	}

	void SquadManager::handleAIEvent(const Event &evt)
	{
		U32 index = evt.filter;

		m_squadHandlers[index].handleAIEvent(evt);
	}

	U32 SquadManager::createSquadHandler()
	{
		auto filter = s_squadFilterMask++;

		SquadHandler newHandler(filter);

		auto result = m_squadHandlers.size();

		m_squadHandlers.push_back(newHandler);

		return result;
	}

	U32 SquadManager::addActor(U16 actorIndex)
	{
		bool found = false;
		U32 index = 0;

		auto sz = m_squadHandlers.size();
		for (U32 i = 0; i < sz; ++i)
		{
			if (m_squadHandlers[i].hasSpace())
			{
				found = true;
				index = i;
				break;
			}
		}

		if (!found)
		{
			index = createSquadHandler();
		}

		m_squadHandlers[index].addActor(actorIndex);
		return index;
	}

	const SquadHandler& SquadManager::getSquadHandler(U32 i) const
	{
		return m_squadHandlers[i];
	}

	SquadHandler& SquadManager::getSquadHandler(U32 i)
	{
		return m_squadHandlers[i];
	}
}