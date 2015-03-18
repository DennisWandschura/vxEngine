#pragma once

class PhysicsAspect;

namespace Component
{
	struct Physics;
}

#include "Condition.h"

class RaycastLessCondition
{
	PhysicsAspect &m_physicsAspect;
	Component::Physics* m_pPhysics;
	F32 m_maxDistance;
	F32 m_cmpDistance;

public:
	RaycastLessCondition(PhysicsAspect &physicsAspect, Component::Physics* componentPhysics, F32 maxDistance, F32 cmpDistance);

	U8 test();
};

class RaycastGreaterCondition
{
	PhysicsAspect &m_physicsAspect;
	Component::Physics* m_pPhysics;
	F32 m_maxDistance;
	F32 m_cmpDistance;

public:
	RaycastGreaterCondition(PhysicsAspect &physicsAspect, Component::Physics* componentPhysics, F32 maxDistance, F32 cmpDistance);

	U8 test();
};