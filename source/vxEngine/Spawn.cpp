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
#include "Spawn.h"

/*namespace YAML
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
}*/