#include "Arrive.h"
#include "ComponentInput.h"
#include "ComponentPhysics.h"

Arrive::Arrive(Component::Physics* pPhysics, Component::Input* pInput)
	:m_pPhysics(pPhysics),
	m_pInput(pInput)
{
}

U8 Arrive::getSteering(SteeringOutput* output)
{
	const F32 targetRadius = 0.25f;
//	const F32 slowRadius = 1.0f;
	//const F32 timeToTarget = 0.1f;

	auto direction = m_targetPosition - m_pPhysics->position;
	auto distance = vx::length(direction);

	if (distance < targetRadius)
	{
		return 0;
	}

	auto velocity = direction / distance * m_maxSpeed;
	output->velocity = velocity;

	/*F32 targetSpeed = m_maxSpeed;

	if (distance <= slowRadius)
	{
		targetSpeed = m_maxSpeed * distance / slowRadius;
	}

	auto targetVelocity = direction / distance;
	//targetVelocity = vx::normalize(targetVelocity);
	targetVelocity *= targetSpeed;

	output->velocity = targetVelocity - m_pInput->velocity;
	output->velocity /= timeToTarget;

	auto l = vx::length(output->velocity);
	if (l > m_maxAcceleration)
	{
		output->velocity = (output->velocity / l) * m_maxAcceleration;
	}*/

	output->angular = 0.0f;

	return 1;
}

void Arrive::setTarget(const vx::float3 &targetPos)
{
	m_targetPosition = targetPos;
}