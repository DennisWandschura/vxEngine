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
#include "Entity.h"

ActionPlayerMove::ActionPlayerMove(EntityHuman* player, f32 moveVelocity, f32 walkModifier)
	:m_player(player),
	m_moveVelocity(moveVelocity),
	m_walkModifier(walkModifier)
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

	f32 walkModifier = (keyboard.m_keys[vx::Input::KEY_SHIFT] != 0) ? m_walkModifier : 1.0f;

	__m128 v = { 0, m_player->m_orientation.x, 0, 0 };
	v = vx::quaternionRotationRollPitchYawFromVector(v);

	// first get movement direction
	__m128 vVelocity = { x_axis, 0, z_axis, 0 };
	// rotate movement direction
	vVelocity = vx::quaternionRotation(vVelocity, v);
	// multiply with max velocity
	auto moveVelocity = m_moveVelocity * walkModifier;
	__m128 vMoveVelocity = { moveVelocity, 0.0f, moveVelocity, 0 };
	vVelocity = _mm_mul_ps(vVelocity, vMoveVelocity);


	auto vy = m_player->m_velocity.y;
	vx::storeFloat4(&m_player->m_velocity, vVelocity);
	m_player->m_velocity.y = vy;

	//_mm_store_ss(&m_inputComponent->velocity.x, vVelocity);
	//m_inputComponent->velocity.z = vVelocity.f[2];
}