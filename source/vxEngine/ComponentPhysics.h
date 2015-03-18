#pragma once

namespace physx
{
	class PxController;
}

#include <vxLib/math/Vector.h>
#include "Component.h"

namespace Component
{
	struct Physics : public Base
	{
		vx::float3 position;
		physx::PxController* pRigidActor{ nullptr };
		vx::float2 orientation;
	};
}