#pragma once

#include <vxLib/math/Vector.h>
#include <vxLib/StringID.h>

enum class PlayerType : U32;

struct Spawn
{
	PlayerType type;
	vx::float3 position;
	// actor id, not used for human
	vx::StringID64 sid;
};

struct SpawnFile
{
	PlayerType type;
	vx::float3 position;
	// actor id, not used for human
	char actor[16];

	SpawnFile()
	{
		actor[0] = '\0';
	}
};