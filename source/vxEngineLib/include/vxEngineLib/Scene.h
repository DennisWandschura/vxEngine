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

class MeshInstance;
struct Waypoint;

#include "SceneBase.h"

struct SceneParams
{
	SceneBaseParams m_baseParams;
	std::unique_ptr<MeshInstance[]> m_pMeshInstances;
	std::unique_ptr<Waypoint[]> m_waypoints;
	u32 m_meshInstanceCount;
	u32 m_waypointCount;

	~SceneParams();
};

class Scene : public SceneBase
{
	std::unique_ptr<MeshInstance[]> m_pMeshInstances;
	std::unique_ptr<Waypoint[]> m_waypoints;
	u32 m_meshInstanceCount{ 0 };
	u32 m_waypointCount{ 0 };

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
	u32 getMeshInstanceCount() const override;
};