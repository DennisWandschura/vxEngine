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

#include "ConverterSceneFileV10.h"
#include <vxLib/string.h>
#include <vxEngineLib/SceneFile.h>
#include <vxEngineLib/MeshInstanceFile.h>
#include <vxEngineLib/Graphics/Light.h>
#include <vxEngineLib/Spawn.h>
#include <vxEngineLib/Actor.h>
#include <vxEngineLib/Joint.h>
#include <vxEngineLib/Waypoint.h>
#include <vxEngineLib/Graphics/LightGeometryProxy.h>

namespace Converter
{
	const u8* SceneFileV10::loadFromMemory(const u8 *ptr, const u8* last, vx::Allocator* allocator, SceneFile* sceneFile)
	{
		ptr = vx::read(sceneFile->m_meshInstanceCount, ptr);
		ptr = vx::read(sceneFile->m_lightCount, ptr);
		ptr = vx::read(sceneFile->m_spawnCount, ptr);
		ptr = vx::read(sceneFile->m_waypointCount, ptr);
		ptr = vx::read(sceneFile->m_jointCount, ptr);
		ptr = vx::read(sceneFile->m_lightGeometryProxyCount, ptr);

		sceneFile->m_pLights = vx::make_unique<Graphics::Light[]>(sceneFile->m_lightCount);
		sceneFile->m_pSpawns = vx::make_unique<SpawnFile[]>(sceneFile->m_spawnCount);

		if (sceneFile->m_meshInstanceCount != 0)
		{
			sceneFile->m_pMeshInstances = vx::make_unique<MeshInstanceFileV8[]>(sceneFile->m_meshInstanceCount);
			ptr = vx::read(sceneFile->m_pMeshInstances.get(), ptr, sceneFile->m_meshInstanceCount);
		}

		ptr = vx::read((u8*)sceneFile->m_pLights.get(), ptr, sizeof(Graphics::Light) *sceneFile->m_lightCount);
		ptr = vx::read((u8*)sceneFile->m_pSpawns.get(), ptr, sizeof(SpawnFile) *sceneFile->m_spawnCount);

		if (sceneFile->m_waypointCount != 0)
		{
			sceneFile->m_waypoints = vx::make_unique<Waypoint[]>(sceneFile->m_waypointCount);

			ptr = vx::read(sceneFile->m_waypoints.get(), ptr, sceneFile->m_waypointCount);
		}

		if (sceneFile->m_jointCount != 0)
		{
			sceneFile->m_joints = vx::make_unique<Joint[]>(sceneFile->m_jointCount);
			ptr = vx::read(sceneFile->m_joints.get(), ptr, sceneFile->m_jointCount);
		}

		if (sceneFile->m_lightGeometryProxyCount != 0)
		{
			sceneFile->m_lightGeometryProxies = vx::make_unique<Graphics::LightGeometryProxy[]>(sceneFile->m_lightGeometryProxyCount);
			ptr = vx::read(sceneFile->m_lightGeometryProxies.get(), ptr, sceneFile->m_lightGeometryProxyCount);
		}

		VX_ASSERT(ptr < last);

		ptr = sceneFile->m_navMesh.load(ptr);

		VX_ASSERT(ptr <= last);

		return ptr;
	}

	u64 SceneFileV10::getCrc(const SceneFile &sceneFile)
	{
		auto navVertexCount = sceneFile.m_navMesh.getVertexCount();
		auto navIndexCount = sceneFile.m_navMesh.getTriangleCount() * 3;
		auto navMeshVertexSize = sizeof(vx::float3) * navVertexCount;
		auto navMeshTriangleSize = sizeof(u16) * navIndexCount;
		auto navMeshSize = navMeshVertexSize + navMeshTriangleSize;

		u32 meshInstanceSize = sizeof(MeshInstanceFileV8) * sceneFile.m_meshInstanceCount;

		auto lightSize = sizeof(Graphics::Light) * sceneFile.m_lightCount;
		auto spawnSize = sizeof(SpawnFile) * sceneFile.m_spawnCount;
		auto jointSize = sizeof(Joint) * sceneFile.m_jointCount;
		auto lightGeomProxySize = sizeof(Graphics::LightGeometryProxy) * sceneFile.m_lightGeometryProxyCount;

		auto totalSize = meshInstanceSize + lightSize + spawnSize + navMeshSize + jointSize + lightGeomProxySize;
		auto ptr = vx::make_unique<u8[]>(totalSize);
		auto current = ptr.get();

		current = vx::write(current, sceneFile.m_pMeshInstances.get(), sceneFile.m_meshInstanceCount);
		current = vx::write(current, sceneFile.m_pLights.get(), sceneFile.m_lightCount);
		current = vx::write(current, sceneFile.m_pSpawns.get(), sceneFile.m_spawnCount);
		current = vx::write(current, sceneFile.m_navMesh.getVertices(), navVertexCount);
		current = vx::write(current, sceneFile.m_navMesh.getTriangleIndices(), navIndexCount);
		current = vx::write(current, sceneFile.m_joints.get(), sceneFile.m_jointCount);
		current = vx::write(current, sceneFile.m_lightGeometryProxies.get(), sceneFile.m_lightGeometryProxyCount);

		auto last = ptr.get() + totalSize;
		VX_ASSERT(current == last);

		return CityHash64((char*)ptr.get(), totalSize);
	}
}