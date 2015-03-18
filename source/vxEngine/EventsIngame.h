#pragma once

#include <vxLib/types.h>

enum class IngameEvent : U16
{
	Level_Started,
	Created_NavGraph,
	Created_InfluenceMap,
	Created_Actor
};