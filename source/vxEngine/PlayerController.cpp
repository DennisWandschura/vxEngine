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
#include "ActionUpdateGpuTransform.h"

#include <vxLib/math/Vector.h>
#include <vxLib/RawInput.h>
#include <vxLib/Graphics/Camera.h>
#include "Entity.h"
#include "input/Keys.h"
#include "EntityAspect.h"
#include "ComponentInput.h"
#include "PhysicsDefines.h"
#include "RenderAspect.h"

PlayerController::PlayerController()
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

void PlayerController::initializePlayer(Component::Input* pPlayerInputComponent, f32 dt, EntityActor* playerEntity, RenderAspect* renderAspect)
{

	m_actions.push_back(vx::make_unique<ActionPlayerLookAround>(pPlayerInputComponent, dt));
	m_actions.push_back(vx::make_unique<ActionPlayerMove>(pPlayerInputComponent, 0.1f));
	m_actions.push_back(vx::make_unique<ActionUpdateGpuTransform>(playerEntity, renderAspect));

	auto &actionLookAround = m_actions[0];
	auto &actionMoveStanding = m_actions[1];
	auto &actionUpdateGpuTransform = m_actions[2];

	m_states.push_back(State());

	auto stateStanding = &m_states[0];
	stateStanding->addAction(actionLookAround.get());
	stateStanding->addAction(actionMoveStanding.get());
	stateStanding->addAction(actionUpdateGpuTransform.get());

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