#include "Seek.h"
#include "ComponentPhysics.h"

Seek::Seek(Component::Physics* pPhysicsComponent, const vx::float3 &target, F32 maxAccel)
	:m_pPhysics(pPhysicsComponent),
	m_targetPosition(target),
	m_maxAcceleration(maxAccel)
{
}

void Seek::setTarget(const vx::float3 &target)
{
	m_targetPosition = target;
}

U8 Seek::getSteering(SteeringOutput* output)
{
	auto dir = m_targetPosition - m_pPhysics->position;
	dir = vx::normalize(dir);

	dir *= m_maxAcceleration;

	output->velocity = dir;
	output->angular = 0.0f;

	return 1;
}