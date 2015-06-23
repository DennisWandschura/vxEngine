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
#include <vxEngineLib/Scene.h>
#include <vxEngineLib/MeshInstance.h>
#include <vxEngineLib/Waypoint.h>
#include <algorithm>
#include <vxEngineLib/Material.h>

SceneParams::~SceneParams()
{

}

Scene::Scene()
	:m_pMeshInstances(),
	m_meshInstanceCount(0)
{
}

Scene::Scene(SceneParams &params)
	: SceneBase(params.m_baseParams),
	m_pMeshInstances(std::move(params.m_pMeshInstances)),
	m_meshInstanceCount(params.m_meshInstanceCount)
{
}

Scene::Scene(Scene &&rhs)
	:SceneBase(std::move(rhs)),
	m_pMeshInstances(std::move(rhs.m_pMeshInstances)),
	m_meshInstanceCount(rhs.m_meshInstanceCount)
{
}

Scene::~Scene()
{

}

void Scene::reset()
{
	SceneBase::reset();
	m_pMeshInstances.reset();
	m_meshInstanceCount = 0;
}

Scene& Scene::operator = (Scene &&rhs)
{
	SceneBase::operator=(std::move(rhs));
	if (this != &rhs)
	{
		m_pMeshInstances = std::move(rhs.m_pMeshInstances);
		m_meshInstanceCount = rhs.m_meshInstanceCount;
	}

	return *this;
}

void Scene::sortMeshInstances()
{
	std::sort(m_pMeshInstances.get(), m_pMeshInstances.get() + m_meshInstanceCount, [&](const MeshInstance &lhs, const MeshInstance &rhs)
	{
		return (lhs.getMeshSid() < rhs.getMeshSid());
	});
}

const MeshInstance* Scene::getMeshInstances() const 
{
	return m_pMeshInstances.get(); 
}

u32 Scene::getMeshInstanceCount() const
{ 
	return m_meshInstanceCount; 
}