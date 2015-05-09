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

#include "ActionPlayerMove.h"
#include <vxLib/Keyboard.h>
#include <vxLib/RawInput.h>
#include "input/Keys.h"
#include "ComponentInput.h"

ActionPlayerMove::ActionPlayerMove()
	:m_inputComponent(nullptr),
	m_moveVelocity(0.0f)
{

}

ActionPlayerMove::ActionPlayerMove(Component::Input* inputComponent, f32 moveVelocity)
	:m_inputComponent(inputComponent),
	m_moveVelocity(moveVelocity)
{

}
ActionPlayerMove::~ActionPlayerMove()
{

}

void ActionPlayerMove::run()
{
	vx::Keyboard keyboard;
	vx::RawInput::getKeyboard(keyboard);

	f32 x_axis = ((s8)(keyboard.m_keys[vx::Input::KEY_D])) - ((s8)(keyboard.m_keys[vx::Input::KEY_A]));
	f32 z_axis = ((s8)(keyboard.m_keys[vx::Input::KEY_S])) - ((s8)(keyboard.m_keys[vx::Input::KEY_W]));

	__m128 v = { 0, m_inputComponent->orientation.x, 0, 0 };
	v = vx::quaternionRotationRollPitchYawFromVector(v);

	// first get movement direction
	__m128 vVelocity = { x_axis, 0, z_axis, 0 };
	// rotate movement direction
	vVelocity = vx::quaternionRotation(vVelocity, v);
	// multiply with max velocity
	__m128 vMoveVelocity = { m_moveVelocity, 0.0f, m_moveVelocity, 0 };
	vVelocity = _mm_mul_ps(vVelocity, vMoveVelocity);

	_mm_store_ss(&m_inputComponent->velocity.x, vVelocity);
	m_inputComponent->velocity.z = vVelocity.f[2];
}