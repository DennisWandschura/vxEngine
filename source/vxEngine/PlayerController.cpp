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
#include "PlayerController.h"
#include "State.h"
#include "ActionPlayerLookAround.h"
#include "ActionPlayerMove.h"
#include <vxLib/memory.h>
#include "ActionPlayerUse.h"
#include "ActionPlaySound.h"
#include "Transition.h"
#include "ConditionPlayerMoving.h"
#include "Entity.h"
#include <vxEngineLib/Locator.h>
#include "ConditionPlayerFalling.h"

PlayerController::PlayerController()
	:m_actionUse(nullptr)
{
}

PlayerController::~PlayerController()
{

}

void PlayerController::initialize(vx::StackAllocator* allocator)
{
	const auto allocSize = 1 KBYTE;
	m_scratchAllocator = vx::StackAllocator(allocator->allocate(allocSize, 8), allocSize);
}

void PlayerController::initializePlayer(f32 dt, EntityHuman* playerEntity, RenderAspectInterface* renderAspect, ComponentActionManager* components)
{
	m_actions.push_back(vx::make_unique<ActionPlayerLookAround>(playerEntity, dt));
	m_actions.push_back(vx::make_unique<ActionPlayerMove>(playerEntity, 0.1f, 0.5f));
	m_actions.push_back(vx::make_unique<ActionPlayerUse>(playerEntity, components));
	m_actions.push_back(vx::make_unique<ActionPlaySound>(vx::make_sid("step1.wav"), Locator::getAudioAspect(), 0.28f, &playerEntity->m_footPosition));

	auto &actionLookAround = m_actions[0];
	auto &actionMoveStanding = m_actions[1];
	auto &actionUse = m_actions[2];
	auto &actionPlaySoundStep = m_actions[3];

	m_actionUse = (ActionPlayerUse*)actionUse.get();

	m_states.push_back(State());
	m_states.push_back(State());
	m_states.push_back(State());

	auto stateStanding = &m_states[0];
	auto stateMoving = &m_states[1];
	auto stateFalling = &m_states[2];

	// state standing
	{
		stateStanding->addAction(actionLookAround.get());
		stateStanding->addAction(actionMoveStanding.get());
		stateStanding->addAction(actionUse.get());

		auto conditionMoving = new ConditionPlayerMoving(playerEntity);
		auto transitionStandingToMoving = new Transition(conditionMoving, stateMoving);

		stateStanding->addTransition(transitionStandingToMoving);
	}

	// state moving
	{
		stateMoving->addAction(actionLookAround.get());
		stateMoving->addAction(actionMoveStanding.get());
		stateMoving->addAction(actionUse.get());
		stateMoving->addAction(actionPlaySoundStep.get());

		auto conditionStopMoving = new ConditionPlayerNotMoving(playerEntity);
		auto transitionMovingToStanding = new Transition(conditionStopMoving, stateStanding);
		stateMoving->addTransition(transitionMovingToStanding);
	}

	// state falling
	{
		stateFalling->addExitAction(actionPlaySoundStep.get());
		auto transitionMovingToFalling = new Transition(new ConditionPlayerFalling(playerEntity), stateFalling);
		auto transitionFallingToMoving = new Transition(new ConditionPlayerNotFalling(playerEntity), stateMoving);

		stateFalling->addTransition(transitionFallingToMoving);
		stateMoving->addTransition(transitionMovingToFalling);
	}

	m_stateMachine.setInitialState(stateStanding);
}

void PlayerController::update()
{
	Action** actions = nullptr;
	u32 count = 0;
	m_stateMachine.update(&actions, &count, &m_scratchAllocator);

	for (u32 i = 0; i < count; ++i)
	{
		actions[i]->run();
	}

	m_scratchAllocator.clear();
}

void PlayerController::onPressedActionKey()
{
	m_actionUse->setKeyDown();
}

void PlayerController::onReleasedActionKey()
{
	m_actionUse->setKeyUp();
}