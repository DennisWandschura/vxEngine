#pragma once

#include <vxLib/types.h>
#include <Windows.h>

namespace vx
{
	namespace Input
	{
		enum Key : U8
		{
			KEY_SHIFT = VK_SHIFT,
			KEY_ESC = VK_ESCAPE,
			KEY_SPACE = VK_SPACE,
			KEY_LEFT = VK_LEFT,
			KEY_UP = VK_UP,
			KEY_RIGHT = VK_RIGHT,
			KEY_DOWN = VK_DOWN,

			KEY_CTRL = VK_CONTROL,

			KEY_0 = 0x30,
			KEY_1 = 0x31,
			KEY_2 = 0x32,
			KEY_3 = 0x33,
			KEY_4 = 0x34,
			KEY_5 = 0x35,
			KEY_6 = 0x36,
			KEY_7 = 0x37,
			KEY_8 = 0x38,
			KEY_9 = 0x39,

			KEY_A = 0x41,
			KEY_B = 0x42,
			KEY_C = 0x43,
			KEY_D = 0x44,

			KEY_S = 0x53,

			KEY_W = 0x57,
			KEY_X = 0x58,
			KEY_Y = 0x59,
			KEY_Z = 0x5A
		};
	}
}