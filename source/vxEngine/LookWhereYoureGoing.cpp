#include "LookWhereYoureGoing.h"
#include "ComponentInput.h"

LookWhereYoureGoing::LookWhereYoureGoing(Component::Input* character)
	:Align(character, nullptr)
{

}

bool LookWhereYoureGoing::getSteering(SteeringOutput* output)
{	Component::Input target;
	m_pTarget = &target;

	if (vx::length(m_pCharacter->velocity) == 0.0f)
		return false;

	target .orientation.x = atan2(-m_pCharacter->velocity.x, m_pCharacter->velocity.z) + 1.5f;

	output->angular = vx::scalarModAngle(target.orientation.x);
	//return Align::getSteering(output);
	return true;
}