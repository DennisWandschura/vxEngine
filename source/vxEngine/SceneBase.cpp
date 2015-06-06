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
#include "SceneBase.h"
#include "Light.h"
#include "Spawn.h"
#include "Actor.h"
#include "copy.h"

SceneBaseParams::~SceneBaseParams()
{

}

SceneBase::SceneBase()
	:m_pLights(),
	m_pSpawns(),
	m_actors()
{
}

SceneBase::SceneBase(SceneBase &&rhs)
	: m_pLights(std::move(rhs.m_pLights)),
	m_materials(std::move(rhs.m_materials)),
	m_meshes(std::move(rhs.m_meshes)),
	m_pSpawns(std::move(rhs.m_pSpawns)),
	m_actors(std::move(rhs.m_actors)),
	m_navMesh(std::move(rhs.m_navMesh)),
	m_lightCount(rhs.m_lightCount),
	m_vertexCount(rhs.m_vertexCount),
	m_indexCount(rhs.m_indexCount),
	m_spawnCount(rhs.m_spawnCount)
{
}

SceneBase::SceneBase(SceneBaseParams &params)
	: m_pLights(std::move(params.m_pLights)),
	m_materials(std::move(params.m_materials)),
	m_meshes(std::move(params.m_meshes)),
	m_pSpawns(std::move(params.m_pSpawns)),
	m_actors(std::move(params.m_actors)),
	m_navMesh(std::move(params.m_navMesh)),
	m_lightCount(params.m_lightCount),
	m_vertexCount(params.m_vertexCount),
	m_indexCount(params.m_indexCount),
	m_spawnCount(params.m_spawnCount)
{
}

SceneBase& SceneBase::operator = (SceneBase &&rhs)
{
	if (this != &rhs)
	{
		m_pLights = std::move(rhs.m_pLights);
		m_materials = std::move(rhs.m_materials);
		m_meshes = std::move(rhs.m_meshes);
		m_pSpawns = std::move(rhs.m_pSpawns);
		m_actors = std::move(rhs.m_actors);
		std::swap(m_navMesh, rhs.m_navMesh);
		m_lightCount = rhs.m_lightCount;
		m_vertexCount = rhs.m_vertexCount;
		m_indexCount = rhs.m_indexCount;
		m_spawnCount = rhs.m_spawnCount;
	}

	return *this;
}

SceneBase::~SceneBase()
{
}

void SceneBase::copy(SceneBase *dst) const
{
#if _VX_EDITOR
	dst->m_pLights = m_pLights;
#else
	copyUniquePtr(&dst->m_pLights, m_pLights, m_lightCount);
#endif

	copySortedVector(&dst->m_materials, m_materials);
	copySortedVector(&dst->m_meshes, m_meshes);
	copySortedVector(&dst->m_actors, m_actors);

	dst->m_pSpawns = vx::make_unique<Spawn[]>(m_spawnCount);
	for (u32 i = 0; i < m_spawnCount; ++i)
	{
		dst->m_pSpawns[i] = m_pSpawns[i];
	}

	m_navMesh.copy(&dst->m_navMesh);

	dst->m_lightCount = m_lightCount;
	dst->m_vertexCount = m_vertexCount;
	dst->m_indexCount = m_indexCount;
	dst->m_spawnCount = m_spawnCount;
}

const Light* SceneBase::getLights() const
{
#if _VX_EDITOR
	return m_pLights.data();
#else
	return m_pLights.get();
#endif
}

u32 SceneBase::getLightCount() const
{
	return m_lightCount;
}

const vx::sorted_vector<vx::StringID, Material*>& SceneBase::getMaterials() const
{
	return m_materials;
}

u32 SceneBase::getMaterialCount() const
{
	return m_materials.size();
}

const vx::sorted_vector<vx::StringID, const vx::MeshFile*>& SceneBase::getMeshes() const
{
	return m_meshes;
}

u32 SceneBase::getVertexCount() const
{
	return m_vertexCount;
}

const Spawn* SceneBase::getSpawns() const
{
	return m_pSpawns.get();
}

u32 SceneBase::getSpawnCount() const
{
	return m_spawnCount;
}

const vx::sorted_vector<vx::StringID, Actor>& SceneBase::getActors() const
{
	return m_actors;
}

NavMesh& SceneBase::getNavMesh()
{
	return m_navMesh;
}

const NavMesh& SceneBase::getNavMesh() const
{
	return m_navMesh;
}