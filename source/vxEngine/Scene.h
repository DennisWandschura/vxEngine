#pragma once

class MeshInstance;
struct Waypoint;

#include "SceneBase.h"

struct SceneParams
{
	SceneBaseParams m_baseParams;
	std::unique_ptr<MeshInstance[]> m_pMeshInstances;
	std::unique_ptr<Waypoint[]> m_waypoints;
	U32 m_meshInstanceCount;
	U32 m_waypointCount;

	~SceneParams();
};

class Scene : public SceneBase
{
	std::unique_ptr<MeshInstance[]> m_pMeshInstances;
	std::unique_ptr<Waypoint[]> m_waypoints;
	U32 m_meshInstanceCount{ 0 };
	U32 m_waypointCount{ 0 };

public:
	Scene();
	Scene(SceneParams &params);

	Scene(const Scene&) = delete;
	Scene(Scene &&rhs);
	~Scene();

	Scene& operator=(const Scene&) = delete;
	Scene& operator=(Scene &&rhs);

	// sorts by material type, then by mesh
	void sortMeshInstances() override;

	const MeshInstance* getMeshInstances() const override;
	U32 getMeshInstanceCount() const override;
};