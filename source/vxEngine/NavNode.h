#pragma once

#include <vxLib/math/Vector.h>

struct NavNode
{
	vx::float3 m_position;
	U16 m_connectionOffset{ 0 };
	U8 m_connectionCount{ 0 };
	U8 m_stuff;
};