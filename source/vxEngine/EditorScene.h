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

class MeshInstance;
struct Waypoint;
class FileEntry;

namespace vx
{
	struct Transform;
}

#include "SceneBase.h"
#include <vector>

struct EditorSceneParams
{
	SceneBaseParams m_baseParams;
	std::vector<MeshInstance> m_meshInstances;
	std::vector<Waypoint> m_waypoints;
	vx::sorted_vector<vx::StringID, std::string> m_materialNames;
	vx::sorted_vector<vx::StringID, std::string> m_meshNames;
	vx::sorted_vector<vx::StringID, std::string> m_actorNames;

	~EditorSceneParams();
};

class EditorScene : public SceneBase
{
	friend class ConverterEditorSceneToSceneFile;

	template<typename T>
	struct SelectableWrapper
	{
		AABB m_bounds;
		T* m_ptr;

		SelectableWrapper() :m_bounds(), m_ptr(nullptr){}
	};

	std::vector<MeshInstance> m_meshInstances;
	std::vector<Waypoint> m_waypoints;
	std::vector<SelectableWrapper<Light>> m_selectableLights;
	std::vector<SelectableWrapper<Spawn>> m_selectableSpawns;

	vx::sorted_vector<vx::StringID, MeshInstance> m_sortedMeshInstances;
	vx::sorted_vector<vx::StringID, std::string> m_materialNames{};
	vx::sorted_vector<vx::StringID, std::string> m_meshNames{};
	vx::sorted_vector<vx::StringID, std::string> m_actorNames{};

	void buildSelectableLights();

public:
	EditorScene();
	EditorScene(EditorScene &&rhs);
	EditorScene(EditorSceneParams &&params);
	~EditorScene();

	EditorScene& operator = (EditorScene &&rhs);

	void sortMeshInstances() override;

	Light* addLight(const Light &light);
	// returns 1 on insert, 0 if already present
	u8 addMesh(vx::StringID sid, const char* name, const vx::MeshFile* pMesh);
	// returns 1 on insert, 0 if already present
	u8 addMaterial(vx::StringID sid, const char* name, Material* pMaterial);
	// returns 1 on insert, 0 if mesh or material is missing
	u8 addMeshInstance(vx::StringID instanceSid, vx::StringID meshSid, vx::StringID materialSid, const vx::Transform &transform);
	void addWaypoint(const Waypoint &wp);

	MeshInstance* findMeshInstance(vx::StringID instanceSid);

	const vx::sorted_vector<vx::StringID, MeshInstance>& getMeshInstancesSortedByName() const { return m_sortedMeshInstances; }
	const MeshInstance* getMeshInstances() const override;
	u32 getMeshInstanceCount() const override;

	const char* getMaterialName(vx::StringID sid) const;
	const char* getMeshName(vx::StringID sid) const;
	const char* getActorName(vx::StringID sid) const;

	Spawn* getSpawn(const Ray &ray);
	Light* getLight(const Ray &ray);

	void updateLightPositions();
};