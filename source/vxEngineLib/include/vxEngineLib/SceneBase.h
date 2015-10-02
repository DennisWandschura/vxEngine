#pragma once
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

class Material;
class MeshInstance;

struct Spawn;
class SceneFile;
struct Waypoint;
struct Joint;

template<typename T>
class Reference;

namespace vx
{
	struct Animation;
	class MeshFile;
}

namespace Graphics
{
	struct LightGeometryProxy;
	struct Light;
}

#include <vxLib/Container/sorted_vector.h>
#include <memory>
#include "NavMesh.h"
#include <vxLib/StringID.h>
#include <vector>

struct SceneBaseParams
{
	std::vector<Graphics::Light> m_lights;
	vx::sorted_vector<vx::StringID, Material*> m_materials;
	vx::sorted_vector<vx::StringID, const vx::MeshFile*> m_meshes;
	vx::sorted_vector<u32, Spawn> m_pSpawns;
	std::vector<Waypoint> m_waypoints;
	vx::sorted_vector<vx::StringID, vx::Animation*> m_animations;
	std::vector<Joint> m_joints;
	std::unique_ptr<Graphics::LightGeometryProxy[]> m_lightGeometryProxies;
	NavMesh m_navMesh;
	u32 m_lightCount;
	u32 m_vertexCount;
	u32 m_indexCount;
	u32 m_spawnCount;
	u32 m_waypointCount;
	u32 m_lightGeometryProxyCount;

	SceneBaseParams();
	~SceneBaseParams();
};

class SceneBase
{
protected:
	std::vector<Graphics::Light> m_lights;
	vx::sorted_vector<vx::StringID, Material*> m_materials;
	vx::sorted_vector<vx::StringID, const vx::MeshFile*> m_meshes;
	vx::sorted_vector<u32, Spawn> m_pSpawns;
	std::vector<Waypoint> m_waypoints;
	vx::sorted_vector<vx::StringID, vx::Animation*> m_animations;
	std::vector<Joint> m_joints;
	std::unique_ptr<Graphics::LightGeometryProxy[]> m_lightGeometryProxies;
	NavMesh m_navMesh;
	u32 m_lightCount;
	u32 m_vertexCount;
	u32 m_indexCount;
	u32 m_spawnCount;
	u32 m_waypointCount;
	u32 m_lightGeometryProxyCount;

	SceneBase();
	SceneBase(const SceneBase &rhs) = delete;
	SceneBase(SceneBaseParams &params);

public:
	SceneBase(SceneBase &&rhs);

	virtual ~SceneBase();

	SceneBase& operator = (const SceneBase&) = delete;
	SceneBase& operator = (SceneBase &&rhs);

	virtual void reset();

	void copy(SceneBase *dst) const;

	virtual void sortMeshInstances() = 0;

	virtual const MeshInstance* getMeshInstances() const = 0;
	virtual u32 getMeshInstanceCount() const = 0;

	const Graphics::Light* getLights() const;
	u32 getLightCount() const;

	Material** getMaterials() const;
	u32 getMaterialCount() const;

	const Material* getMaterial(const vx::StringID &sid) const;

	const vx::sorted_vector<vx::StringID, const vx::MeshFile*>& getMeshes() const;
	u32 getVertexCount() const;

	const vx::MeshFile* getMesh(const vx::StringID &sid);

	const Spawn* getSpawns() const;
	u32 getSpawnCount() const;

	vx::Animation** getAnimations() const;
	u32 getAnimationCount() const;

	NavMesh& getNavMesh();
	const NavMesh& getNavMesh() const;

	const Waypoint* getWaypoints() const;
	u32 getWaypointCount() const;

	const Joint* getJoints() const;
	u32 getJointCount() const;

	const Graphics::LightGeometryProxy* getLightGeometryProxies() const;
	u32 getLightGeometryProxyCount() const;

	void getLightGeometryProxyBounds(u32 index, const AABB &bounds);
};