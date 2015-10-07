#include "Entity.h"
#include "PhysicsDefines.h"
#include <vxEngineLib/Locator.h>
#include "PhysicsAspect.h"
#include <characterkinematic/PxController.h>
#include <vxEngineLib/Graphics/RenderAspectInterface.h>
#include <vxEngineLib/GpuFunctions.h>
#include <PxRigidDynamic.h>
#include <vxEngineLib/AudioAspectInterface.h>

void EntityHuman::update(f32 dt, RenderAspectInterface* renderAspect, AudioAspectInterface* audioAspect)
{
	auto physicsAspect = Locator::getPhysicsAspect();
	const vx::ivec4 velocityMask = { (s32)0xffffffff, 0, (s32)0xffffffff, 0 };
	const __m128 vGravity = { 0, g_gravity * dt, 0, 0 };

	__m128 qRotation = { m_orientation.y, m_orientation.x, 0, 0 };
	qRotation = vx::quaternionRotationRollPitchYawFromVector(qRotation);
	vx::storeFloat4(&m_qRotation, qRotation);

	__m128 vVelocity = vx::loadFloat4(m_velocity);

	vVelocity = _mm_and_ps(vVelocity, velocityMask);
	vVelocity = _mm_add_ps(vVelocity, vGravity);

	physicsAspect->move(vVelocity, dt, m_controller);

	auto footPosition = m_controller->getFootPosition();
	auto position = m_controller->getPosition();

	auto diff = footPosition.y - m_footPosition.y;
	s32 isFalling = (diff <= -0.3f);
	m_state ^= (-isFalling ^ m_state) & (1 << (s32)EntityHuman::State::Falling);

	m_position.x = position.x;
	m_position.y = position.y;
	m_position.z = position.z;
	m_footPosition.x = footPosition.x;
	m_footPosition.y = footPosition.y;
	m_footPosition.z = footPosition.z;

	RenderUpdateCameraData data;
	data.position = { position.x, position.y, position.z, 1.0 };
	data.quaternionRotation = { qRotation.m128_f32[0], qRotation.m128_f32[1], qRotation.m128_f32[2], qRotation.m128_f32[3] };

	renderAspect->queueUpdateCamera(data);
	audioAspect->setListenerPosition(vx::loadFloat3(m_position));
	audioAspect->setListenerRotation(qRotation);
}

void EntityActor::update(f32 dt, PhysicsAspect* physicsAspect, vx::TransformGpu* transforms, u32* indices, u32 index)
{
	const vx::ivec4 velocityMask = { (s32)0xffffffff, 0, (s32)0xffffffff, 0 };
	const __m128 vGravity = { 0, g_gravity * dt, 0, 0 };

	__m128 qRotation = { m_orientation.y, m_orientation.x, 0, 0 };
	qRotation = vx::quaternionRotationRollPitchYawFromVector(qRotation);
	vx::storeFloat4(&m_qRotation, qRotation);

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

	indices[index] = m_gpuIndex;
	transforms[index].translation = m_position;
	transforms[index].scaling = 1.0f;
	transforms[index].packedQRotation = GpuFunctions::packQRotation(qRotation);
}

void EntityDynamic::update(f32 dt, vx::TransformGpu* transforms, u32* indices, u32* index)
{
	auto transform = m_rigidDynamic->getGlobalPose();
	vx::float3 newPosition;
	newPosition.x = transform.p.x;
	newPosition.y = transform.p.y;
	newPosition.z = transform.p.z;

	vx::float4 newRotation;
	newRotation.x = transform.q.x;
	newRotation.y = transform.q.y;
	newRotation.z = transform.q.z;
	newRotation.w = transform.q.w;

	auto qRotationOld = vx::loadFloat4(m_qRotation);
	auto qRotationNew = _mm_loadu_ps(&transform.q.x);

	auto diffRot = _mm_sub_ps(qRotationNew, qRotationOld);
	auto distRot = vx::dot4(diffRot, diffRot);

	auto diffPos = newPosition - m_position;
	auto distPos = diffPos.x * diffPos.x + diffPos.y * diffPos.y + diffPos.z * diffPos.z;
	auto cmpPos = (distPos != 0.0);

	auto cmpRotation = (distRot.m128_f32[0] != 0.0);
	if (cmpRotation || cmpPos)
	{
		auto currentIndex = (*index)++;
		indices[currentIndex] = m_gpuIndex;
		transforms[currentIndex].translation = newPosition;
		transforms[currentIndex].scaling = 1.0f;
		transforms[currentIndex].packedQRotation = GpuFunctions::packQRotation(qRotationNew);
	}

	m_position = newPosition;
	vx::storeFloat4(&m_qRotation, qRotationNew);
}