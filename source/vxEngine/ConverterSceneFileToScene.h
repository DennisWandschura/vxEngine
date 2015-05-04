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
#pragma once

class Scene;
class SceneFile;
class Material;
class MeshInstance;
struct Actor;

namespace vx
{
	class Mesh;
}

#include <vxLib/Container/sorted_array.h>
#include <vxLib/StringID.h>

class ConverterSceneFileToScene
{
	struct CreateSceneMeshInstancesDesc
	{
		const SceneFile *sceneFile;
		const vx::sorted_array<vx::StringID, vx::Mesh*> *sortedMeshes;
		const vx::sorted_array<vx::StringID, Material*> *sortedMaterials;
		MeshInstance* pMeshInstances;
		vx::sorted_vector<vx::StringID, const vx::Mesh*>* sceneMeshes;
		vx::sorted_vector<vx::StringID, Material*>* sceneMaterials;
	};

	struct CreateSceneActorsDesc
	{
		const SceneFile *sceneFile;
		const vx::sorted_array<vx::StringID, vx::Mesh*> *sortedMeshes;
		const vx::sorted_array<vx::StringID, Material*> *sortedMaterials;
		vx::sorted_vector<vx::StringID, Actor>* sceneActors;
		vx::sorted_vector<vx::StringID, const vx::Mesh*>* sceneMeshes; 
		vx::sorted_vector<vx::StringID, Material*>* sceneMaterials;
	};

	static bool createSceneMeshInstances(const CreateSceneMeshInstancesDesc &desc);
	static bool createSceneActors(const CreateSceneActorsDesc &desc);

public:
	static bool convert(const vx::sorted_array<vx::StringID, vx::Mesh*> *sortedMeshes, const vx::sorted_array<vx::StringID, Material*> *sortedMaterials, const SceneFile &sceneFile, Scene* scene);
};