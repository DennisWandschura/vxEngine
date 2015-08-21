#include "LookWhereYoureGoing.h"
#include "Entity.h"

LookWhereYoureGoing::LookWhereYoureGoing(EntityActor* character)
	:Align(character, nullptr)
{

}

bool LookWhereYoureGoing::getSteering(SteeringOutput* output)
{	
	EntityActor target;
	m_pTarget = &target;

	vx::float4a len = vx::length3(vx::loadFloat4(m_pCharacter->m_velocity));
	if (len.x == 0.0f)
		return false;

	target.m_orientation.x = atan2(m_pCharacter->m_velocity.x, m_pCharacter->m_velocity.z) - 1.5f;

	output->angular = vx::scalarModAngle(target.m_orientation.x);
	//return Align::getSteering(output);
	return true;
}