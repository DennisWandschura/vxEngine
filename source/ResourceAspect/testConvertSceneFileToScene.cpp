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
#include <vxEngineLib/SceneFile.h>
#include <vxResourceAspect/ConverterSceneFileToScene.h>
#include <vxEngineLib/Actor.h>
#include <vxLib/file/FileHandle.h>
#include <vxEngineLib/Light.h>
#include "ConverterSceneFile.h"
#include <vxEngineLib/MeshInstanceFile.h>
#include <vxEngineLib/Joint.h>
#include <vxEngineLib/Spawn.h>
#include <vxEngineLib/Waypoint.h>

struct Comparer
{
	template<typename T>
	static bool compare(const T &lhs, const T &rhs)
	{
		return lhs == rhs;
	}

	template<typename T, typename U, typename Cmp>
	static bool compare(const T &lhs, const U &rhs, Cmp cmp)
	{
		return cmp(lhs, rhs);
	}

	template<typename T1, typename T2, typename U, typename Cmp>
	static bool compare(const vx::sorted_vector<T1, T2> &lhs, const U* rhs, u32 count, Cmp cmp)
	{
		if (count != lhs.size())
			return false;

		for (u32 i = 0; i < count; ++i)
		{
			if (!cmp(lhs[i], rhs[i]))
				return false;
		}

		return true;
	}

	template<typename T>
	static bool compare(const T* lhs, u32 count1, const T* rhs, u32 count2)
	{
		if (count1 != count2)
			return false;

		for (u32 i = 0; i < count; ++i)
		{
			if (lhs[i] != rhs[i])
				return false;
		}

		return true;
	}

	template<typename T, typename Cmp>
	static bool compare(const T* lhs, u32 count1, const T* rhs, u32 count2, Cmp cmp)
	{
		if (count1 != count2)
			return false;

		for (u32 i = 0; i < count1; ++i)
		{
			if (!cmp(lhs[i], rhs[i]))
				return false;
		}

		return true;
	}
};

bool testLights(const Converter::SceneFile &sceneFile, const Scene &scene)
{
	printf("testLights start\n");

	auto fileLightCount = sceneFile.getLightCount();
	auto fileLights = sceneFile.getLights();

	auto sceneLightCount = scene.getLightCount();
	auto sceneLights = scene.getLights();

	bool result = Comparer::compare(fileLights, fileLightCount, sceneLights, sceneLightCount, [](const Light &lhs, const Light &rhs)
	{
		auto result = (memcmp(&lhs, &rhs, sizeof(Light)) == 0);
		if (!result)
		{
			printf("lights not equal\n");
		}

		return result;
	});

	if (result)
	{
		printf("testLights success\n");
	}

	return result;
}

bool testActors(const Converter::SceneFile &sceneFile, const Scene &scene)
{
	printf("testActors start\n");

	auto fileActors = sceneFile.getActors();
	auto fileActorCount = sceneFile.getActorCount();
	auto &sceneActors = scene.getActors();

	bool result = Comparer::compare(sceneActors, fileActors, fileActorCount, [](const Actor &lhs, const ActorFile &rhs)
	{
		if (vx::FileHandle(rhs.m_material).m_sid != lhs.m_material)
		{
			printf("wrong material sid\n");
			return false;
		}

		if (vx::FileHandle(rhs.m_mesh).m_sid != lhs.m_mesh)
		{
			printf("wrong mesh sid\n");
			return false;
		}

		return true;
	});

	if (result)
	{
		printf("testActors success\n");
	}

	return result;
}

bool testConvertSceneFileToScene
(
	const ResourceManager<vx::MeshFile>* meshManager,
	const ResourceManager<Material>* materialManager,
	SceneFile &&sceneFile
	)
{
	printf("testConvertSceneFileToScene start\n");

	Scene scene;
	if (!Converter::SceneFileToScene::convert(meshManager, materialManager, &sceneFile, &scene))
	{
		printf("testConvertSceneFileToScene: error converting\n");
		return false;
	}

	Converter::SceneFile testSceneFile(std::move(sceneFile));

	if (!testLights(testSceneFile, scene))
	{
		printf("testActors failure\n");
		return false;
	}

	if (!testActors(testSceneFile, scene))
	{
		printf("testActors failure\n");
		return false;
	}

	printf("testConvertSceneFileToScene success\n");
	return true;

	/*

	scene->getJointCount();
	scene->getJoints();

	scene->getMaterialCount();
	scene->getMaterials();

	scene->getMeshes();

	scene->getMeshInstanceCount();
	scene->getMeshInstances();
	scene->getNavMesh();
	scene->getSpawnCount();
	scene->getSpawns();
	scene->getWaypointCount();
	scene->getWaypoints();
	*/
}