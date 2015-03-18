#pragma once

namespace Component
{
	struct Physics;
}

#include <vxLib/math/Vector.h>

struct Align
{
	Component::Physics* m_pCharacter;
	Component::Physics* m_pTarget;
	F32 m_maxAngularAcceleration;
	F32 m_maxRotation;
	F32 m_targetRadius;
	F32 m_slowRadius;

	void update(vx::float2* orientation);
};