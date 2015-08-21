#include "Entity.h"
#include "PhysicsDefines.h"
#include <vxEngineLib/Locator.h>
#include "PhysicsAspect.h"
#include <characterkinematic/PxController.h>

void EntityHuman::update(f32 dt)
{
	auto physicsAspect = Locator::getPhysicsAspect();
	const vx::ivec4 velocityMask = { (s32)0xffffffff, 0, (s32)0xffffffff, 0 };
	const __m128 vGravity = { 0, g_gravity * dt, 0, 0 };

	__m128 qRotation = { m_orientation.y, m_orientation.x, 0, 0 };
	qRotation = vx::quaternionRotationRollPitchYawFromVector(qRotation);
	vx::storeFloat4(&m_qRotation, qRotation);

	//__m128 vVelocity = { p->velocity.x, 0, p->velocity.z, 0.0f };
	__m128 vVelocity = vx::loadFloat4(m_velocity);

	vVelocity = _mm_and_ps(vVelocity, velocityMask);
	vVelocity = _mm_add_ps(vVelocity, vGravity);

	physicsAspect->move(vVelocity, dt, m_controller);

	auto footPosition = m_controller->getFootPosition();
	auto position = m_controller->getPosition();

	m_position.x = position.x;
	m_position.y = position.y;
	m_position.z = position.z;
	m_footPositionY = footPosition.y;
}

void EntityActor::update(f32 dt)
{
	auto physicsAspect = Locator::getPhysicsAspect();
	const vx::ivec4 velocityMask = { (s32)0xffffffff, 0, (s32)0xffffffff, 0 };
	const __m128 vGravity = { 0, g_gravity * dt, 0, 0 };

	__m128 qRotation = { m_orientation.y, m_orientation.x, 0, 0 };
	qRotation = vx::quaternionRotationRollPitchYawFromVector(qRotation);
	vx::storeFloat4(&m_qRotation, qRotation);

	//__m128 vVelocity = { p->velocity.x, 0, p->velocity.z, 0.0f };
	__m128 vVelocity = vx::loadFloat4(m_velocity);

	vVelocity = _mm_and_ps(vVelocity, velocityMask);
	vVelocity = _mm_add_ps(vVelocity, vGravity);

	physicsAspect->move(vVelocity, dt, m_controller);

	auto footPosition = m_controller->getFootPosition();
	auto position = m_controller->getPosition();

	m_position.x = position.x;
	m_position.y = position.y;
	m_position.z = position.z;
	m_footPositionY = footPosition.y;
}