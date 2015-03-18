#pragma once

namespace Component
{
	struct Physics;
}

#include "Steering.h"

struct Seek : public Steering
{
	Component::Physics* m_pPhysics;
	vx::float3 m_targetPosition;
	F32 m_maxAcceleration;

	Seek(Component::Physics* pPhysicsComponent, const vx::float3 &target, F32 maxAccel);

	void setTarget(const vx::float3 &target);

	U8 getSteering(SteeringOutput* output) override;
};