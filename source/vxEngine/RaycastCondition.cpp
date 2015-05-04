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