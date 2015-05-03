#pragma once

namespace YAML
{
	class Node;
}

#include <vxLib/math/Vector.h>
#include <vxLib/StringID.h>
#include <vector>

enum class PlayerType : U32;

struct Spawn
{
	PlayerType type;
	vx::float3 position;
	// actor id, not used for human
	vx::StringID sid;
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

	static std::vector<SpawnFile> loadFromYaml(const YAML::Node &n);
	static YAML::Node saveToYaml(const SpawnFile* spawns, U32 count);
};