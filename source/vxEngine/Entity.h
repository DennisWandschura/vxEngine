#pragma once

#include <vxLib/types.h>

const U32 g_maxEntities = 100u;

struct Entity
{
	U16 input{ 0 };
	U16 physics{ 0 };
	U16 render{ 0 };
	U16 actor{ 0 };
};