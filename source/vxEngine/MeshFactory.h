#pragma once

namespace vx
{
	class Mesh;
}

#include <vector>

class MeshFactory
{
	static std::string s_meshDir;
	static const std::string s_extension;

public:
	static void setDataDir(const std::string &dataDir);
};