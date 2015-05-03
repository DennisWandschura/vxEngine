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
	vx::sorted_vector<vx::StringID, char[32]> m_materialNames;
	vx::sorted_vector<vx::StringID, char[32]> m_meshNames;
	vx::sorted_vector<vx::StringID, char[32]> m_actorNames;

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
	vx::sorted_vector<vx::StringID, char[32]> m_materialNames{};
	vx::sorted_vector<vx::StringID, char[32]> m_meshNames{};
	vx::sorted_vector<vx::StringID, char[32]> m_actorNames{};

public:
	EditorScene();
	EditorScene(EditorScene &&rhs);
	EditorScene(EditorSceneParams &&params);
	~EditorScene();

	EditorScene& operator = (EditorScene &&rhs);

	void sortMeshInstances() override;

	// returns 1 on insert, 0 if already present
	U8 addMesh(vx::StringID sid, const char* name, const vx::Mesh* pMesh);
	// returns 1 on insert, 0 if already present
	U8 addMaterial(vx::StringID sid, const char* name, Material* pMaterial);
	// returns 1 on insert, 0 if mesh or material is missing
	U8 addMeshInstance(vx::StringID instanceSid, vx::StringID meshSid, vx::StringID materialSid, const vx::Transform &transform);
	void addWaypoint(const Waypoint &wp);

	MeshInstance* findMeshInstance(vx::StringID instanceSid);

	const vx::sorted_vector<vx::StringID, MeshInstance>& getMeshInstancesSortedByName() const { return m_sortedMeshInstances; }
	const MeshInstance* getMeshInstances() const override;
	U32 getMeshInstanceCount() const override;

	const char* getMaterialName(vx::StringID sid) const;
	const char* getMeshName(vx::StringID sid) const;
	const char* getActorName(vx::StringID sid) const;

	Spawn* getSpawn(const Ray &ray);
	Light* getLight(const Ray &ray);
};