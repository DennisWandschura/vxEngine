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

#include "ComponentActorManager.h"
#include "ActionManager.h"
#include <vxLib/Allocator/StackAllocator.h>
#include "ComponentActor.h"
#include "StateMachine.h"

#include "Transition.h"
#include "ConditionActorFollowingPath.h"
#include "ConditionActorHasPath.h"
#include "ActionFollowPath.h"
#include "ActionSetFollowPath.h"
#include "ActionPrintText.h"
#include "ActionActorCreatePath.h"
#include "State.h"

ComponentActorManager::ComponentActorManager()
	:m_pool()
{

}

ComponentActorManager::~ComponentActorManager()
{
	
}

void ComponentActorManager::initialize(u32 capacity, vx::StackAllocator* pAllocator)
{
	m_pool.initialize(pAllocator->allocate(sizeof(Component::Actor) * capacity, __alignof(Component::Actor)), capacity);
}

void ComponentActorManager::shutdown()
{
	m_pool.clear();
	m_pool.release();
}

Component::Actor* ComponentActorManager::createComponent(u16 entityIndex, EntityActor* entity, const QuadTree* quadTree, u16* index)
{
	auto pActor = m_pool.createEntry(index);

	pActor->entityIndex = entityIndex;
	pActor->m_data = vx::make_unique<Component::ActorData>();
	pActor->m_busy = 0;
	pActor->m_followingPath = 0;

	ActionFollowPath* actionFollowPath = new ActionFollowPath(entity, pActor, quadTree, 0.2f, 2.0f);
	ActionSetFollowPath* actionSetFollowPath = new ActionSetFollowPath(actionFollowPath, pActor->m_data.get());

	ActionActorCreatePath* actionActorCreatePath = new ActionActorCreatePath(pActor);

	State* waitingState = new State();
	State* movingState = new State();

	waitingState->addAction(actionActorCreatePath);

	ConditionActorHasPath* conditionActorHasPath = new ConditionActorHasPath(pActor->m_data.get());
	ConditionActorNotFollowingPath* conditionActorNotFollowingPath = new ConditionActorNotFollowingPath(pActor);

	Transition* transitionWaitingToMoving = new Transition(conditionActorHasPath, movingState);
	waitingState->addTransition(transitionWaitingToMoving);
	transitionWaitingToMoving->addAction(actionSetFollowPath);
	transitionWaitingToMoving->addAction(actionFollowPath);

	Transition* transitionMovingToWaiting = new Transition(conditionActorNotFollowingPath, waitingState);
	movingState->addTransition(transitionMovingToWaiting);

	pActor->m_stateMachine.setInitialState(waitingState);

	return pActor;
}

void ComponentActorManager::update(ActionManager* actionManager, vx::StackAllocator* scratchAllocator)
{
	auto p = m_pool.first();
	while (p != nullptr)
	{
		auto marker = scratchAllocator->getMarker();

		Action** actions = 0;
		u32 count = 0;
		p->m_stateMachine.update(&actions, &count, scratchAllocator);

		actionManager->scheduleActions(actions, count);

		scratchAllocator->clear(marker);

		p = m_pool.next_nocheck(p);
	}
}

Component::Actor& ComponentActorManager::operator[](u32 i)
{
	return m_pool[i];
}

const Component::Actor& ComponentActorManager::operator[](u32 i) const
{
	return m_pool[i];
}