#pragma once

#include <vxLib/types.h>

struct NavConnection
{
	union
	{
		struct
		{
			U16 m_fromNode;
			U16 m_toNode;
		};
		U16 nodes[2];
	};
	F32 m_cost{0.0f};
};