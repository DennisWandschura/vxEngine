#pragma once

namespace Component
{
	struct Input;
}

#include <vxLib/math/Vector.h>

struct SteeringOutput
{
	vx::float3 velocity;
	F32 angular{0.0f};
};

class Steering
{
public:
	virtual U8 getSteering(SteeringOutput* output) = 0;
};