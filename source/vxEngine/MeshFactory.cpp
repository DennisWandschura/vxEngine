#include "MeshFactory.h"
#include <vxLib/math/Vector.h>
#include <vxLib\Graphics\Mesh.h>

std::string MeshFactory::s_meshDir{"data/mesh/"};
const std::string MeshFactory::s_extension{".mesh"};

void MeshFactory::setDataDir(const std::string &dataDir)
{
	s_meshDir = dataDir + "mesh/";
}