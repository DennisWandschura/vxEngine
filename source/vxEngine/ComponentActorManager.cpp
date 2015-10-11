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
#include "ActionPlaySound.h"
#include "Entity.h"
#include <vxEngineLib/Locator.h>
#include "ConditionCanSeePlayer.h"
#include "ActionActorStop.h"

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

Component::Actor* ComponentActorManager::createComponent(const CreateActorComponentDesc &desc)
{
	auto actorComponent = m_pool.createEntry(desc.componentIndex);

	actorComponent->entityIndex = desc.entityIndex;
	actorComponent->m_data = vx::make_unique<Component::ActorData>();
	actorComponent->m_busy = 0;
	actorComponent->m_followingPath = 0;

	ActionFollowPath* actionFollowPath = new ActionFollowPath(desc.entity, actorComponent, desc.quadTree, 0.2f, 2.0f);
	ActionSetFollowPath* actionSetFollowPath = new ActionSetFollowPath(actionFollowPath, actorComponent->m_data.get());

	ActionActorCreatePath* actionActorCreatePath = new ActionActorCreatePath(actorComponent);

	auto actionPlaySound = new ActionPlaySound(vx::make_sid("step1.wav"), Locator::getAudioAspect(), 0.28f, &desc.entity->m_position);

	State* waitingState = new State();
	State* stateMoving = new State();

	State* stateCanSeePlayer= new State();

	waitingState->addAction(actionActorCreatePath);

	ConditionActorHasPath* conditionActorHasPath = new ConditionActorHasPath(actorComponent->m_data.get());
	ConditionActorNotFollowingPath* conditionActorNotFollowingPath = new ConditionActorNotFollowingPath(actorComponent);

	Transition* transitionWaitingToMoving = new Transition(conditionActorHasPath, stateMoving);
	waitingState->addTransition(transitionWaitingToMoving);
	transitionWaitingToMoving->addAction(actionSetFollowPath);
	transitionWaitingToMoving->addAction(actionFollowPath);

	Transition* transitionMovingToWaiting = new Transition(conditionActorNotFollowingPath, waitingState);
	stateMoving->addTransition(transitionMovingToWaiting);
	stateMoving->addAction(actionPlaySound);

	ConditionCanSeePlayer* conditionCanSeePlayer = new ConditionCanSeePlayer(desc.human, desc.entity, desc.fovRad, desc.maxViewDistance);
	Transition* transitionMovingToCanSeePlayer = new Transition(conditionCanSeePlayer, stateCanSeePlayer);
	transitionMovingToCanSeePlayer->addAction(new ActionActorStop(desc.entity, actorComponent, actionFollowPath));
	stateMoving->addTransition(transitionMovingToCanSeePlayer);

	ConditionCanNotSeePlayer* conditionCanNotSeePlayer = new ConditionCanNotSeePlayer(desc.human, desc.entity, desc.fovRad, desc.maxViewDistance);
	Transition* transitionCanSeePlayerToMoving = new Transition(conditionCanNotSeePlayer, stateMoving);
	stateCanSeePlayer->addTransition(transitionCanSeePlayerToMoving);

	actorComponent->m_stateMachine.setInitialState(waitingState);

	return actorComponent;
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