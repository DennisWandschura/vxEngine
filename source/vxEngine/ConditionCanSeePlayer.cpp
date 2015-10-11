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

#include "ConditionCanSeePlayer.h"
#include "Entity.h"
#include <vxEngineLib/Locator.h>
#include "PhysicsAspect.h"
#include <characterkinematic/PxController.h>
#include <PxRigidBody.h>

ConditionCanSeePlayer::ConditionCanSeePlayer(EntityHuman* player, EntityActor* actor, f32 fovRad, f32 maxViewDistance)
	:m_player(player),
	m_actor(actor),
	m_fov(0.0f),
	m_maxViewDistance(maxViewDistance)
{
	// 0 ... 1
	auto fov = (fovRad * 0.5f) / vx::VX_PI;
	m_fov = fov * (-2.0f) + 1.0f;
}

ConditionCanSeePlayer::~ConditionCanSeePlayer()
{

}

u8 ConditionCanSeePlayer::test() const
{
	auto checkConcreteType = [](const physx::PxRigidActor* actor)
	{
		return (actor->getConcreteType() == physx::PxConcreteType::eRIGID_DYNAMIC);
	};

	const __m128 forwardDir{ 0, 0, -1, 0 };

	__m128 playerPosition = vx::loadFloat4((vx::float4*)&m_player->m_position);
	__m128 actorPosition = vx::loadFloat4((vx::float4*)&m_actor->m_position);
	__m128 qRotation = vx::loadFloat4(&m_actor->m_qRotation);

	auto directionToPlayer = _mm_sub_ps(playerPosition, actorPosition);
	__m128 distanceToPlayer = vx::length3(directionToPlayer);

	u8 result = 0;
	if (distanceToPlayer.m128_f32[0] <= m_maxViewDistance)
	{
		directionToPlayer = _mm_div_ps(directionToPlayer, distanceToPlayer);

		auto viewDirection = vx::quaternionRotation(forwardDir, qRotation);

		auto dp = vx::dot3(viewDirection, directionToPlayer);
		if (dp.m128_f32[0] >= m_fov)
		{
			auto physicsAspect = Locator::getPhysicsAspect();

			auto rigidDynamic = m_actor->m_controller->getActor();
			PhysicsHitData hitData;
			if (physicsAspect->raycast_staticDynamicFilter(actorPosition, directionToPlayer, m_maxViewDistance, rigidDynamic, &hitData))
			{
				if (checkConcreteType(hitData.actor))
				{
					result = 1;
				}
			}
		}

		// dp = 1: fov angle = 0°, 0
		// dp = 0: fov angle = 45°, PI/2
		// dp = -1: fov angle = 180°, PI
	}

	return result;
}

ConditionCanNotSeePlayer::ConditionCanNotSeePlayer(EntityHuman* player, EntityActor* actor, f32 fovRad, f32 maxViewDistance)
	:ConditionCanSeePlayer(player, actor, fovRad, maxViewDistance)
{

}

ConditionCanNotSeePlayer::~ConditionCanNotSeePlayer()
{

}

u8 ConditionCanNotSeePlayer::test() const
{
	u8 result = (ConditionCanSeePlayer::test() == 0) ? 1 : 0;

	return result;
}