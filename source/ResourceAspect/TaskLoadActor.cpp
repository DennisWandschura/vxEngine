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

#include "TaskLoadActor.h"
#include <vxEngineLib/ActorFile.h>
#include <vxResourceAspect/ResourceManager.h>
#include <vxResourceAspect/ResourceAspect.h>
#include <vxEngineLib/CpuTimer.h>

enum class TaskLoadActor::State : u32 { LoadFile, CheckDependencies};

TaskLoadActor::TaskLoadActor(TaskLoadActorDesc &&desc)
	:TaskLoadFile(std::move(desc.m_fileNameWithPath), desc.m_actorResManager->getScratchAllocator(), desc.m_actorResManager->getScratchAllocatorMutex(), std::move(desc.evt)),
	m_fileName(std::move(desc.m_fileName)),
	m_sid(desc.m_sid),
	m_actorResManager(desc.m_actorResManager),
	m_resourceAspect(desc.m_resourceAspect),
	m_actor(),
	m_state(State::LoadFile)
{

}

TaskLoadActor::~TaskLoadActor()
{

}

bool TaskLoadActor::loadActorFile(vx::FileHandle* handleMeshOut, vx::FileHandle* handleMaterialOut)
{
	u32 fileSize = 0;
	managed_ptr<u8[]> fileData;
	if (!loadFromFile(&fileData, &fileSize))
		return false;

	const u8* dataBegin = nullptr;
	u32 dataSize = 0;
	u64 crc = 0;
	u32 version = 0;
	if (!readAndCheckHeader(fileData.get(), fileSize, &dataBegin, &dataSize, &crc, &version))
	{
		return false;
	}

	ActorFile actorFile(version);
	actorFile.loadFromMemory(dataBegin, dataSize, nullptr);

	vx::FileHandle handleMesh(actorFile.getMesh());
	vx::FileHandle handleMaterial(actorFile.getMaterial());

	m_actor.m_mesh = handleMesh.m_sid;
	m_actor.m_material = handleMaterial.m_sid;
	m_actor.m_fov = actorFile.getFovRad();

	*handleMeshOut = handleMesh;
	*handleMaterialOut = handleMaterial;

	return true;
}

bool TaskLoadActor::checkDependenciesAndLoad(const vx::FileHandle &handleMesh, const vx::FileHandle &handleMaterial)
{
	std::vector<Event> events;
	events.reserve(2);

	bool result = true;
	auto material = m_resourceAspect->getMaterial(handleMaterial.m_sid);
	if (material == nullptr)
	{
		auto evt = Event::createEvent();

		vx::Variant arg;
		arg.u64 = 0;
		m_resourceAspect->requestLoadFile(vx::FileEntry(handleMaterial.m_string, vx::FileType::Material), arg, evt);

		events.push_back(evt);

		result = false;
	}

	auto mesh = m_resourceAspect->getMesh(handleMesh.m_sid);
	if (mesh == nullptr)
	{
		auto evt = Event::createEvent();

		vx::Variant arg;
		arg.u64 = 0;
		m_resourceAspect->requestLoadFile(vx::FileEntry(handleMesh.m_string, vx::FileType::Mesh), arg, evt);

		events.push_back(evt);

		result = false;
	}

	if (!events.empty())
	{
		setEventList(&events);
	}

	return result;
}

bool TaskLoadActor::checkDependencies()
{
	auto material = m_resourceAspect->getMaterial(m_actor.m_material);
	auto mesh = m_resourceAspect->getMesh(m_actor.m_mesh);

	return (material != nullptr && mesh != nullptr);
}

TaskReturnType TaskLoadActor::runImpl()
{
	TaskReturnType result = TaskReturnType::Success;

	switch (m_state)
	{
	case State::LoadFile:
	{
		vx::FileHandle handleMesh, handleMaterial;
		if (!loadActorFile(&handleMesh, &handleMaterial))
		{
			result = TaskReturnType::Failure;
			break;
		}

		if (!checkDependenciesAndLoad(handleMesh, handleMaterial))
		{
			result = TaskReturnType::Retry;
			m_state = State::CheckDependencies;
		}
	}break;
	case State::CheckDependencies:
	{
		if (!checkDependencies())
		{
			result = TaskReturnType::Retry;
			break;
		}
		else
		{
			m_actorResManager->insertEntry(m_sid, std::move(m_fileName), std::move(m_actor));
		}

	}break;
	default:
		break;
	}

	return result;
}