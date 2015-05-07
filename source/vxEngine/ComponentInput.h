/*
The MIT License (MIT)

Copyright (c) 2015 Dennis Wandschura

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#pragma once

#include "Component.h"
#include <vxLib/math/Vector.h>

namespace Component
{
	struct Input : public Base
	{
		enum Action : u16
		{
			Action_None = 0,
			Action_Left = 1,
			Action_Right = 2,
			Action_Forward = 3,
			Action_Backward = 4,
			Action_Run = 5,
			Action_Crouch = 6
		};

		enum State : u8
		{
			State_None = 0,
			State_Crouch = 1
		};

		//u16 action{ 0 };
		//u8 state{ 0 };
		vx::float3 velocity{ 0, 0, 0 };
		//vx::g_halfPi
		vx::float2 orientation{ vx::VX_PIDIV2, 0 };
	};
}