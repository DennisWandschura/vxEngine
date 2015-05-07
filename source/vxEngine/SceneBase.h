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

class Material;
class MeshInstance;

struct Light;
struct Spawn;
struct Actor;
class SceneFile;

namespace vx
{
	class Mesh;
}

#include <vxLib/Container/sorted_vector.h>
#include <memory>
#include "NavMesh.h"
#include <vxLib/StringID.h>

struct SceneBaseParams
{
#if _VX_EDITOR
	std::vector<Light> m_pLights;
#else
	std::unique_ptr<Light[]> m_pLights;
#endif
	vx::sorted_vector<vx::StringID, Material*> m_materials;
	vx::sorted_vector<vx::StringID, const vx::Mesh*> m_meshes;
	std::unique_ptr<Spawn[]> m_pSpawns;
	vx::sorted_vector<vx::StringID, Actor> m_actors;
	NavMesh m_navMesh;
	u32 m_lightCount;
	u32 m_vertexCount;
	u32 m_indexCount;
	u32 m_spawnCount;

	~SceneBaseParams();
};

class SceneBase
{
protected:
#if _VX_EDITOR
	std::vector<Light> m_pLights;
#else
	std::unique_ptr<Light[]> m_pLights;
#endif
	vx::sorted_vector<vx::StringID, Material*> m_materials{};
	vx::sorted_vector<vx::StringID, const vx::Mesh*> m_meshes{};
	std::unique_ptr<Spawn[]> m_pSpawns;
	vx::sorted_vector<vx::StringID, Actor> m_actors;
	NavMesh m_navMesh{};
	u32 m_lightCount{ 0 };
	u32 m_vertexCount{ 0 };
	u32 m_indexCount{ 0 };
	u32 m_spawnCount{ 0 };

	SceneBase();
	SceneBase(SceneBaseParams &params);

public:
	SceneBase(SceneBase &&rhs);
	SceneBase(const SceneBase&) = delete;

	virtual ~SceneBase();

	SceneBase& operator = (const SceneBase&) = delete;
	SceneBase& operator = (SceneBase &&rhs);

	virtual void sortMeshInstances() = 0;

	virtual const MeshInstance* getMeshInstances() const = 0;
	virtual u32 getMeshInstanceCount() const = 0;

	const Light* getLights() const;
	u32 getLightCount() const;

	const vx::sorted_vector<vx::StringID, Material*>& getMaterials() const;
	u32 getMaterialCount() const;

	const vx::sorted_vector<vx::StringID, const vx::Mesh*>& getMeshes() const;
	u32 getVertexCount() const;

	const Spawn* getSpawns() const;
	u32 getSpawnCount() const;

	const vx::sorted_vector<vx::StringID, Actor>& getActors() const;

	NavMesh& getNavMesh();
	const NavMesh& getNavMesh() const;
};