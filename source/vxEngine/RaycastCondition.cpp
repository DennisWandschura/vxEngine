#include "RaycastCondition.h"
#include "PhysicsAspect.h"
#include "ComponentPhysics.h"

RaycastLessCondition::RaycastLessCondition(PhysicsAspect &physicsAspect, Component::Physics* componentPhysics, F32 maxDistance, F32 cmpDistance)
	:m_physicsAspect(physicsAspect),
	m_pPhysics(componentPhysics),
	m_maxDistance(maxDistance),
	m_cmpDistance(cmpDistance)
{
}

U8 RaycastLessCondition::test()
{
	vx::float3 dir = { m_pPhysics->orientation.x, m_pPhysics->orientation.y, 0.0f };
	dir = vx::normalize(dir);

	vx::float3 hit;
	if (m_physicsAspect.raycast_static(m_pPhysics->position, dir, m_maxDistance, &hit) != 0)
	{
		auto d = hit - m_pPhysics->position;
		F32 distance = vx::length(d);

		if (distance <= m_cmpDistance)
			return 1;
	}

	return 0;
}

RaycastGreaterCondition::RaycastGreaterCondition(PhysicsAspect &physicsAspect, Component::Physics* componentPhysics, F32 maxDistance, F32 cmpDistance)
	:m_physicsAspect(physicsAspect),
	m_pPhysics(componentPhysics),
	m_maxDistance(maxDistance),
	m_cmpDistance(cmpDistance)
{
}

U8 RaycastGreaterCondition::test()
{
	vx::float3 dir = { m_pPhysics->orientation.x, m_pPhysics->orientation.y, 0.0f };
	dir = vx::normalize(dir);

	U8 result = 1;

	vx::float3 hit;
	if (m_physicsAspect.raycast_static(m_pPhysics->position, dir, m_maxDistance, &hit) != 0)
	{
		auto d = hit - m_pPhysics->position;
		F32 distance = vx::length(d);

		if (distance <= m_cmpDistance)
			result = 0;
	}

	return result;
}