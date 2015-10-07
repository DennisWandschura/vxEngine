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
#pragma once

namespace vx
{
	struct Keyboard;
	struct Mouse;
	class Camera;
	class MessageManager;
}

namespace Component
{
	struct Input;
}

struct EntityHuman;
class EntityAspect;
class RenderAspectInterface;
class ComponentActionManager;
class ActionPlayerUse;

#include <vxLib/types.h>
#include "StateMachine.h"
#include <memory>
#include <vxLib/Allocator/StackAllocator.h>
#include <vector>

class PlayerController
{
	StateMachine m_stateMachine;
	ActionPlayerUse* m_actionUse;
	vx::StackAllocator m_scratchAllocator;

	std::vector<std::unique_ptr<Action>> m_actions;
	std::vector<State> m_states;

public:
	PlayerController();
	~PlayerController();

	void initialize(vx::StackAllocator* allocator);
	void initializePlayer(f32 dt, EntityHuman* playerEntity, RenderAspectInterface* renderAspect, ComponentActionManager* components);

	void update();

	void onPressedActionKey();
	void onReleasedActionKey();
};