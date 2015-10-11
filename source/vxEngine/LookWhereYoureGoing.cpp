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

	auto velocity = vx::loadFloat4(&m_pCharacter->m_velocity);
	vx::float4a len = vx::length3(velocity);
	if (len.x == 0.0f)
		return false;

	auto vz = _mm_load_ss(&m_pCharacter->m_velocity.z);
	auto orientation = vx::atan2(velocity, vz);

	target.m_orientation.x = orientation.m128_f32[0];// -1.5f;//atan2(m_pCharacter->m_velocity.x, m_pCharacter->m_velocity.z) - 1.5f;

	output->angular = vx::scalarModAngle(target.m_orientation.x);
	//return Align::getSteering(output);
	return true;
}