#include "ActorFactory.h"
#include "ComponentActor.h"
#include "DecisionHasDestination.h"
#include "ActionRequestPath.h"
#include "ActionFollowPath.h"
#include "ActionGoToPosition.h"
#include "NavGraph.h"
#include "ComponentPhysics.h"
#include "AStar.h"
#include "State.h"
#include "Transition.h"
#include "ConditionHasDestination.h"
#include "PoolAllocator.h"

void ActorFactory::create(const NavGraph* navGraph, Component::Actor *p, Component::Input *pInput, Component::Physics *pPhysics, F32 halfHeight, vx::PoolAllocator* pAllocator)
{
	U32 flags = Component::Actor::WaitingForOrders;

	auto capacity = navGraph->getNodeCount();

	p->flags = flags;
	p->data = std::make_unique<ActorData>();
	p->data->path = std::move(vx::ManagedArray<vx::float3>(capacity, pAllocator));
/*	p->data->destination.x = 7.0f;
	p->data->destination.y = 1.5f;
	p->data->destination.z = -5.5f;*/
	p->halfHeight = halfHeight;
	//p->data->path = std::move(nodes);

	ConditionHasDestination* hasDest = new ConditionHasDestination(p);
	ConditionHasNoDestination* hasNoDest = new ConditionHasNoDestination(p);

	State* requestPatrolState = new State();
	State* patrolState = new State();

	Transition* patrolToRequest = new Transition(hasNoDest, requestPatrolState);
	Transition* requestToPatrol = new Transition(hasDest, patrolState);

	{
		ActionFollowPath* pActionTrue1 = new ActionFollowPath(pInput, pPhysics, p);
		pActionTrue1->updateTargetPosition();
		patrolState->setAction(pActionTrue1);
		patrolState->addTransition(patrolToRequest);
	}

	{
		ActionRequestPath* pActionRequestPath = new ActionRequestPath(p);
		requestPatrolState->addTransition(requestToPatrol);
		requestPatrolState->setAction(pActionRequestPath);
	}

	p->m_sm.addState(requestPatrolState);
	p->m_sm.setInitialState(patrolState);
	//p->m_root = std::make_unique<DecisionHasDestination>(p, pActionTrue1, pActionFalse);
}