#pragma once

#include <vxLib/types.h>

enum class IngameEvent : U16
{
	Level_Started,
	Created_NavGraph,
	Created_InfluenceMap,
	Create_Actor,
	Created_Actor,
	Created_Actor_GPU
};