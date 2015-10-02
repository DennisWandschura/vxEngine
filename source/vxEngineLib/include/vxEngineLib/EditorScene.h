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


struct Waypoint;
class FileEntry;

namespace vx
{
	struct Transform;
}

namespace Converter
{
	class EditorSceneToSceneFile;
}

#include "SceneBase.h"
#include <vector>

namespace Editor
{
	class MeshInstance;

	struct SceneParams
	{
		SceneBaseParams m_baseParams;
		vx::sorted_vector<vx::StringID, MeshInstance> m_meshInstances;
		std::vector<Waypoint> m_waypoints;
		vx::sorted_vector<vx::StringID, std::string> m_materialNames;
		vx::sorted_vector<vx::StringID, std::string> m_meshNames;
		vx::sorted_vector<vx::StringID, std::string> m_meshInstanceNames;

		~SceneParams();
	};

	class Scene : public SceneBase
	{
		friend Converter::EditorSceneToSceneFile;

		template<typename T>
		struct SelectableWrapper
		{
			AABB m_bounds;
			T* m_ptr;

			SelectableWrapper() :m_bounds(), m_ptr(nullptr){}
		};

		struct SelectableWrapperIndex
		{
			AABB m_bounds;
			u32 m_index;

			SelectableWrapperIndex() :m_bounds(), m_index(0){}
		};

		vx::sorted_vector<vx::StringID, MeshInstance> m_meshInstances;
		std::vector<SelectableWrapper<Graphics::Light>> m_selectableLights;
		std::vector<std::pair<AABB, u32>> m_selectableSpawns;
		std::vector<SelectableWrapper<Waypoint>> m_selectableWaypoints;
		std::vector<SelectableWrapperIndex> m_selectableJoints;

		vx::sorted_vector<vx::StringID, std::string> m_materialNames;
		vx::sorted_vector<vx::StringID, std::string> m_meshNames;
		vx::sorted_vector<vx::StringID, std::string> m_animationNames;
		u32 m_spawnHumanId;

		void buildSelectableLights();
		void buildSelectableWaypoints();
		void buildSelectableSpawns();
		void buildSelectableJoints();

		const ::MeshInstance* getMeshInstances() const override { return nullptr; }

	public:
		Scene();
		Scene(const Scene &rhs) = delete;
		Scene(Scene &&rhs);
		Scene(SceneParams &&params);
		~Scene();

		Scene& operator = (const Scene &rhs) = delete;
		Scene& operator = (Scene &&rhs);

		void reset() override;
		void copy(Scene* dst) const;

		void sortMeshInstances() override;

		void removeUnusedMaterials();
		void removeUnusedMeshes();

		Graphics::Light* addLight(const Graphics::Light &light);
		// returns 1 on insert, 0 if already present
		u8 addMesh(vx::StringID sid, const char* name, const vx::MeshFile* mesh);
		// returns 1 on insert, 0 if already present
		u8 addMaterial(vx::StringID sid, const char* name, Material* material);
		// returns 1 on insert, 0 if mesh or material is missing
		void addWaypoint(const vx::float3 &position);
		void removeWaypoint(const vx::float3 &position);
		void addAnimation(const vx::StringID &sid, std::string &&name);

		const MeshInstance* getMeshInstance(const vx::StringID &sid) const;
		MeshInstance* getMeshInstance(const vx::StringID &sid);
		const MeshInstance* getMeshInstancesEditor() const;
		u32 getMeshInstanceCount() const override;
		const vx::sorted_vector<vx::StringID, MeshInstance>& getSortedMeshInstances() const;
		void setMeshInstancePosition(const vx::StringID &sid, const vx::float3 &p);
		void setMeshInstanceRotation(const vx::StringID &sid, const vx::float4 &q);

		const char* getMeshInstanceName(const vx::StringID &sid) const;
		const char* getMaterialName(const vx::StringID &sid) const;
		const char* getMeshName(const vx::StringID &sid) const;
		const char* getAnimationName(const vx::StringID &sid) const;

		u32 getAnimationCount() const;
		const char* getAnimationNameIndex(u32 i) const;
		u64 getAnimationSidIndex(u32 i) const;

		vx::StringID createMeshInstance();
		void removeMeshInstance(const vx::StringID &sid);
		bool renameMeshInstance(const vx::StringID &sid, const char* newName);

		void addSpawn(Spawn &&spawn);
		u32 getSpawnId(const Ray &ray) const;
		u32 getSpawnHumanId() const;
		const Spawn* getSpawn(u32 id) const;
		Spawn* getSpawn(u32 id);
		void setSpawnActor(u32 id, const vx::StringID &sid);

		void setSpawnPosition(u32 id, const vx::float3 &position);
		void setSpawnType(u32 id, u32 type);

		Graphics::Light* getLight(const Ray &ray);
		Graphics::Light* getLight(u32 i);

		void updateLightPositions();

		void addJoint(const Joint &joint);
		void eraseJoint(u32 i);
		Joint* getJoint(const Ray &ray, u32* index);
		void setJointPosition0(u32 index, const vx::float3 &p);
		void setJointPosition1(u32 index, const vx::float3 &p);
		void setJointBody0(u32 index, u64 sid);
		void setJointBody1(u32 index, u64 sid);
		void setJointRotation0(u32 index, const vx::float4 &q);
		void setJointRotation1(u32 index, const vx::float4 &q);
		void setJointLimit(u32 index, u32 enabled, f32 limitMin, f32 limitMax);

		void addLightGeometryProxy(const AABB &bounds);
		void setLightGeometryProxyBounds(u32 index, const AABB &bounds);
	};
}