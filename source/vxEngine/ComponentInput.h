#pragma once

#include "Component.h"
#include <vxLib/math/Vector.h>

namespace Component
{
	struct Input : public Base
	{
		enum Action : U16
		{
			Action_None = 0,
			Action_Left = 1,
			Action_Right = 2,
			Action_Forward = 3,
			Action_Backward = 4,
			Action_Run = 5,
			Action_Crouch = 6
		};

		enum State : U8
		{
			State_None = 0,
			State_Crouch = 1
		};

		//U16 action{ 0 };
		//U8 state{ 0 };
		vx::float3 velocity{ 0, 0, 0 };
		//vx::g_halfPi
		vx::float2 orientation{ vx::VX_PIDIV2, 0 };
	};
}