#pragma once

template<typename T>
class ResourceManager;

namespace vx
{
	class MeshFile;
}

class Material;
class SceneFile;
class Scene;

extern bool testConvertSceneFileToScene
(
	const ResourceManager<vx::MeshFile>* meshManager,
	const ResourceManager<Material>* materialManager,
	SceneFile &&sceneFile
	);