#pragma once

namespace Component
{
	struct Input;
}

struct EntityActor;

#include "Steering.h"

class Arrive : public Steering
{
	EntityActor* m_pEntity{ nullptr };
	Component::Input* m_pInput{ nullptr };
	vx::float3 m_targetPosition;
	//F32 m_maxAcceleration{ 0.5f };
	F32 m_maxSpeed{0.2f};

public:
	explicit Arrive(EntityActor* actor, Component::Input* pInput);

	U8 getSteering(SteeringOutput* output) override;

	void setTarget(const vx::float3 &targetPos);

	Component::Input* getInputComponent() const;
};