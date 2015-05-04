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
#include <vxLib/Allocator/PoolAllocator.h>
#include "EntityFactoryDescription.h"

void ActorFactory::create(const EntityFactoryDescription &description, vx::PoolAllocator* pAllocator)
{
	U32 flags = Component::Actor::WaitingForOrders;

	auto capacity = description.navGraph->getNodeCount();

	description.p->flags = flags;
	description.p->data = std::make_unique<ActorData>();
	description.p->data->path = std::move(vx::array<vx::float3>(capacity, pAllocator));
/*	p->data->destination.x = 7.0f;
	p->data->destination.y = 1.5f;
	p->data->destination.z = -5.5f;*/
	description.p->halfHeight = description.halfHeight;
	//p->data->path = std::move(nodes);

	ConditionHasDestination* hasDest = new ConditionHasDestination(description.p);
	ConditionHasNoDestination* hasNoDest = new ConditionHasNoDestination(description.p);

	State* requestPatrolState = new State();
	State* patrolState = new State();

	Transition* patrolToRequest = new Transition(hasNoDest, requestPatrolState);
	Transition* requestToPatrol = new Transition(hasDest, patrolState);

	{
		ActionFollowPath* pActionTrue1 = new ActionFollowPath(description.pInput, description.entity, description.p);
		pActionTrue1->updateTargetPosition();
		patrolState->setAction(pActionTrue1);
		patrolState->addTransition(patrolToRequest);
	}

	{
		ActionRequestPath* pActionRequestPath = new ActionRequestPath(description.p);
		requestPatrolState->addTransition(requestToPatrol);
		requestPatrolState->setAction(pActionRequestPath);
	}

	description.p->m_sm.addState(requestPatrolState);
	description.p->m_sm.setInitialState(patrolState);
	//p->m_root = std::make_unique<DecisionHasDestination>(p, pActionTrue1, pActionFalse);
}