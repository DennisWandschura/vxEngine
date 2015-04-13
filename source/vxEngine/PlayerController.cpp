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

void PlayerController::updatePlayerHuman(Entity* pPlayer, EntityAspect &entityAspect)
{
	if (pPlayer == nullptr)
		return;

	auto physComp = entityAspect.getComponentPhysics(pPlayer->physics);

	__m128 quaternionRotation = { physComp.orientation.y, physComp.orientation.x, 0, 0 };
	quaternionRotation = vx::QuaternionRotationRollPitchYawFromVector(quaternionRotation);

	RenderUpdateCameraData data;
	data.position = vx::loadFloat(physComp.position);
	data.quaternionRotation = quaternionRotation;

	m_pRenderAspect->queueUpdateCamera(data);
}

void PlayerController::handleKeyboard(Entity* pPlayer, const vx::Keyboard &keyboard, EntityAspect &entityAspect)
{
	if (pPlayer == nullptr)
		return;

	/*U32 actionMask = (keyboard.m_keys[vx::Input::KEY_A] << Component::Input::Action_Left) |
		(keyboard.m_keys[vx::Input::KEY_D] << Component::Input::Action_Right) |
		(keyboard.m_keys[vx::Input::KEY_W] << Component::Input::Action_Forward) |
		(keyboard.m_keys[vx::Input::KEY_S] << Component::Input::Action_Backward) |
		(keyboard.m_keys[vx::Input::KEY_SHIFT] << Component::Input::Action_Run) |
		(keyboard.m_keys[vx::Input::KEY_CTRL] << Component::Input::Action_Crouch);*/

	F32 x_axis = ((I8)(keyboard.m_keys[vx::Input::KEY_D])) - ((I8)(keyboard.m_keys[vx::Input::KEY_A]));
	F32 z_axis = ((I8)(keyboard.m_keys[vx::Input::KEY_S])) - ((I8)(keyboard.m_keys[vx::Input::KEY_W]));

	auto &input = entityAspect.getComponentInput(pPlayer->input);

	__m128 v = { 0, input.orientation.x, 0, 0 };
	v = vx::QuaternionRotationRollPitchYawFromVector(v);

	// first get movement direction
	__m128 vVelocity = { x_axis, 0, z_axis, 0 };
	// rotate movement direction
	vVelocity = vx::Vector3Rotate(vVelocity, v);
	// multiply with max velocity
	vVelocity = _mm_mul_ps(vVelocity, g_vMoveVelocity);

	input.velocity.x = vVelocity.f[0];
	input.velocity.z = vVelocity.f[2];
}

void PlayerController::handleMouse(Entity* pPlayer, const vx::Mouse &mouse, const F32 dt, EntityAspect &entityAspect)
{
	const F32 angleMax = 1.4f;
	const F32 angleMin = -1.4f;

	if (pPlayer == nullptr)
		return;

	auto &input = entityAspect.getComponentInput(pPlayer->input);
	input.orientation.x = input.orientation.x + (-(F32)mouse.m_relative.x * 0.5f * dt);
	input.orientation.y = input.orientation.y + (-(F32)mouse.m_relative.y * 0.5f * dt);

	input.orientation.x = vx::scalarModAngle(input.orientation.x);
	input.orientation.y = fminf(fmaxf(input.orientation.y, angleMin), angleMax);
}

void PlayerController::keyPressed(U16 key, EntityAspect &entityAspect)
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