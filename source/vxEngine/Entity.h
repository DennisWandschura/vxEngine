#pragma once

namespace physx
{
	class PxController;
}

#include <vxLib/types.h>
#include <vxLib/math/Vector.h>

const U32 g_maxEntities = 100u;

struct EntityActor
{
	physx::PxController* pRigidActor{ nullptr };
	vx::float3 position;
	F32 footPositionY;
	vx::float2 orientation;

	U16 input{ 0 };
	U16 render{ 0 };
	U16 actor{ 0 };
};