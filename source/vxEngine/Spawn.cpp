#include "Spawn.h"
#include "yamlHelper.h"

namespace YAML
{
	template<>
	struct convert < Spawn >
	{
		static bool decode(const YAML::Node &node, Spawn &data)
		{
			data.type = (PlayerType)node["type"].as<U32>();
			data.position = node["position"].as<vx::float3>();
			return true;
		}

		static Node encode(const Spawn &rhs)
		{
			Node node;
			node["type"] = (U32)rhs.type;
			node["position"] = rhs.position;

			return node;
		}
	};

	template<>
	struct convert < SpawnFile >
	{
		static bool decode(const YAML::Node &node, SpawnFile &data)
		{
			data.type = (PlayerType)node["type"].as<U32>();
			data.position = node["position"].as<vx::float3>();
			std::string actor = node["actor"].as<std::string>();

			strncpy_s(data.actor, actor.data(), actor.size());

			return true;
		}

		static Node encode(const SpawnFile &rhs)
		{
			Node node;
			node["type"] = (U32)rhs.type;
			node["position"] = rhs.position;
			node["actor"] = std::string(rhs.actor);

			return node;
		}
	};
}

std::vector<SpawnFile> SpawnFile::loadFromYaml(const YAML::Node &n)
{
	return n.as<std::vector<SpawnFile>>();
}

YAML::Node SpawnFile::saveToYaml(const SpawnFile* spawns, U32 count)
{
	YAML::Node spawnsNode;
	for (auto i = 0u; i < count; ++i)
	{
		spawnsNode[i] = spawns[i];
	}

	return spawnsNode;
}