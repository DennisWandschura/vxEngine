#include "Align.h"
#include "ComponentPhysics.h"

void Align::update(vx::float2* orientation)
{
	const F32 timeToTarget = 0.1f;

	auto rotation = m_pTarget->orientation - m_pCharacter->orientation;

	rotation.x = vx::scalarModAngle(rotation.x);

	auto rotSz = abs(rotation.x);

	if (rotSz < m_targetRadius)
		return;

	F32 targetRotation = m_maxRotation;
	if (rotSz <= m_slowRadius)
	{
		targetRotation = m_maxRotation * rotSz / m_slowRadius;
	}

	targetRotation *= rotation.x / rotSz;

	F32 angular = targetRotation - m_pCharacter->orientation.x;
	angular /= timeToTarget;

	F32 angularAccel = abs(angular);
	if (angularAccel > m_maxAngularAcceleration)
	{
		angular /= angularAccel;
		angular *= m_maxAngularAcceleration;
	}

	orientation->x = angular;
}