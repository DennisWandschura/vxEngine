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
#include <vxLib/math/Vector.h>
#include <vxLib/RawInput.h>
#include <vxLib/Graphics/Camera.h>
#include "Entity.h"
#include "Keys.h"
#include "EntityAspect.h"
#include "ComponentPhysics.h"
#include "ComponentInput.h"
#include "PhysicsDefines.h"
#include "RenderAspect.h"

PlayerController::PlayerController(RenderAspect* renderAspect)
	:m_pRenderAspect(renderAspect)
{
}

void PlayerController::updatePlayerHuman(EntityActor* pPlayer, EntityAspect &entityAspect)
{
	if (pPlayer == nullptr)
		return;

	//auto physComp = entityAspect.getComponentPhysics(pPlayer->physics);

	__m128 quaternionRotation = { pPlayer->orientation.y, pPlayer->orientation.x, 0, 0 };
	quaternionRotation = vx::quaternionRotationRollPitchYawFromVector(quaternionRotation);

	RenderUpdateCameraData data;
	data.position = vx::loadFloat(pPlayer->position);
	data.quaternionRotation = quaternionRotation;

	m_pRenderAspect->queueUpdateCamera(data);
}

void PlayerController::handleKeyboard(EntityActor* pPlayer, const vx::Keyboard &keyboard, EntityAspect &entityAspect)
{
	if (pPlayer == nullptr)
		return;

	/*u32 actionMask = (keyboard.m_keys[vx::Input::KEY_A] << Component::Input::Action_Left) |
		(keyboard.m_keys[vx::Input::KEY_D] << Component::Input::Action_Right) |
		(keyboard.m_keys[vx::Input::KEY_W] << Component::Input::Action_Forward) |
		(keyboard.m_keys[vx::Input::KEY_S] << Component::Input::Action_Backward) |
		(keyboard.m_keys[vx::Input::KEY_SHIFT] << Component::Input::Action_Run) |
		(keyboard.m_keys[vx::Input::KEY_CTRL] << Component::Input::Action_Crouch);*/

	f32 x_axis = ((s8)(keyboard.m_keys[vx::Input::KEY_D])) - ((s8)(keyboard.m_keys[vx::Input::KEY_A]));
	f32 z_axis = ((s8)(keyboard.m_keys[vx::Input::KEY_S])) - ((s8)(keyboard.m_keys[vx::Input::KEY_W]));

	auto &input = entityAspect.getComponentInput(pPlayer->input);

	__m128 v = { 0, input.orientation.x, 0, 0 };
	v = vx::quaternionRotationRollPitchYawFromVector(v);

	// first get movement direction
	__m128 vVelocity = { x_axis, 0, z_axis, 0 };
	// rotate movement direction
	vVelocity = vx::quaternionRotation(vVelocity, v);
	// multiply with max velocity
	vVelocity = _mm_mul_ps(vVelocity, g_vMoveVelocity);

	input.velocity.x = vVelocity.f[0];
	input.velocity.z = vVelocity.f[2];
}

void PlayerController::handleMouse(EntityActor* pPlayer, const vx::Mouse &mouse, const f32 dt, EntityAspect &entityAspect)
{
	const f32 angleMax = 1.4f;
	const f32 angleMin = -1.4f;

	if (pPlayer == nullptr)
		return;

	auto &input = entityAspect.getComponentInput(pPlayer->input);
	input.orientation.x = input.orientation.x + (-(f32)mouse.m_relative.x * 0.5f * dt);
	input.orientation.y = input.orientation.y + (-(f32)mouse.m_relative.y * 0.5f * dt);

	input.orientation.x = vx::scalarModAngle(input.orientation.x);
	input.orientation.y = fminf(fmaxf(input.orientation.y, angleMin), angleMax);
}

void PlayerController::keyPressed(u16 key, EntityAspect &entityAspect)
{
	VX_UNREFERENCED_PARAMETER(key);
	VX_UNREFERENCED_PARAMETER(entityAspect);
	/*if (key == VK_SPACE)
	{
	auto &input = entityAspect.getComponentInput(m_pPlayer->input);
	auto isJumping = (input.direction >> Component::Input::IS_JUMPING) & 0x1;
	if (isJumping == 0)
	{
	input.direction |= (1 << Component::Input::JUMP);
	}
	}*/
}