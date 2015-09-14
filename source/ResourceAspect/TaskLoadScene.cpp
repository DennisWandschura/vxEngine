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
#include "TaskLoadScene.h"
#include <vxResourceAspect/SceneFactory.h>
#include <vxEngineLib/FileEntry.h>
#include <vxLib/File/File.h>
#include <vxLib/ScopeGuard.h>
#include "TaskLoadMesh.h"
#include <vxEngineLib/Event.h>
#include <vxEngineLib/TaskManager.h>
#include "TaskLoadAnimation.h"
#include "TaskLoadMaterial.h"

namespace TaskLoadSceneCpp
{
	const u32 g_allocSize = 64 KBYTE;
}

TaskLoadScene::TaskLoadScene(TaskLoadSceneDesc &&rhs)
	:Task(std::move(rhs.m_evt)),
	m_filenameWithPath(std::move(rhs.m_filenameWithPath)),
	m_meshManager(rhs.m_meshManager),
	m_materialManager(rhs.m_materialManager),
	m_animationManager(rhs.m_animationManager),
	m_textureManager(rhs.m_textureManager),
	m_scene(rhs.m_scene),
	m_scratchAllocator((u8*)_aligned_malloc(TaskLoadSceneCpp::g_allocSize, 16), TaskLoadSceneCpp::g_allocSize),
	m_taskManager(rhs.m_taskManager),
	m_directories(rhs.m_directories),
	m_flipImage(rhs.m_flipImage),
	m_editor(rhs.m_editor)
{
}

TaskLoadScene::~TaskLoadScene()
{
	auto p = m_scratchAllocator.release();
	_aligned_free(p);
}

bool TaskLoadScene::loadFile(u8** outData, u32* outFileSize)
{
	vx::File file;
	if (!file.open(m_filenameWithPath.c_str(), vx::FileAccess::Read))
	{
		return false;
	}

	auto fileSize = file.getSize();
	VX_ASSERT(fileSize == static_cast<u32>(fileSize));

	auto fileData = m_scratchAllocator.allocate(fileSize, 16);
	memset(fileData, 0, fileSize);

	file.read(fileData, static_cast<u32>(fileSize));
	*outFileSize = static_cast<u32>(fileSize);
	*outData = fileData;

	return true;
}

void TaskLoadScene::createTaskLoadMesh(const vx::FileEntry &it, std::vector<Event>* events)
{
	auto fileName = it.getString();

	char fileNameWithPath[64];
	auto returnCode = sprintf_s(fileNameWithPath, "%s%s", m_directories.meshDir, fileName);
	if (returnCode == -1)
	{
		VX_ASSERT(false);
	}

	auto evt = Event::createEvent();

	TaskLoadMeshDesc desc;
	desc.m_fileNameWithPath = std::string(fileNameWithPath);
	desc.m_filename = fileName;
	desc.m_meshManager = m_meshManager;
	desc.m_sid = it.getSid();
	desc.evt = evt;

	auto task = new TaskLoadMesh(std::move(desc));
	m_taskManager->pushTask(task);
	events->push_back(evt);
}

void TaskLoadScene::createTaskLoadMaterial(const vx::FileEntry &it, std::vector<Event>* events)
{
	auto fileName = it.getString();

	char fileNameWithPath[64];
	auto returnCode = sprintf_s(fileNameWithPath, "%s%s", m_directories.materialDir, fileName);
	if (returnCode == -1)
	{
		VX_ASSERT(false);
	}

	auto evt = Event::createEvent();

	TaskLoadMaterialDesc desc;
	desc.m_fileNameWithPath = std::string(fileNameWithPath);
	desc.m_filename = fileName;
	desc.m_materialManager = m_materialManager;
	desc.m_textureManager = m_textureManager;
	desc.m_sid = it.getSid();
	desc.evt = evt;
	desc.m_taskManager = m_taskManager;
	desc.m_textureFolder = m_directories.textureDir;
	desc.m_flipImage = m_flipImage;

	auto task = new TaskLoadMaterial(std::move(desc));
	m_taskManager->pushTask(task);
	events->push_back(evt);
}

void TaskLoadScene::createTaskLoadAnimation(const vx::FileEntry &it, std::vector<Event>* events)
{
	auto fileName = it.getString();

	char fileNameWithPath[64];
	auto returnCode = sprintf_s(fileNameWithPath, "%s%s", m_directories.animDir, fileName);
	if (returnCode == -1)
	{
		VX_ASSERT(false);
	}

	auto evt = Event::createEvent();

	TaskLoadAnimationDesc desc;
	desc.m_fileNameWithPath = std::string(fileNameWithPath);
	desc.m_filename = fileName;
	desc.m_animationManager = m_animationManager;
	desc.m_sid = it.getSid();
	desc.evt = evt;

	auto task = new TaskLoadAnimation(std::move(desc));
	m_taskManager->pushTask(task);
	events->push_back(evt);
}

TaskReturnType TaskLoadScene::runImpl()
{
	u8* fileData = nullptr;
	u32 fileSize = 0;
	auto marker = m_scratchAllocator.getMarker();
	SCOPE_EXIT
	{
		m_scratchAllocator.clear(marker);
	};

	if(!loadFile(&fileData, &fileSize))
	{
		printf("Error loading scene file\n");
		return TaskReturnType::Failure;
	}

	vx::sorted_vector<vx::StringID, vx::FileEntry> missingFiles;

	Factory::CreateSceneDesc factoryDesc;
	factoryDesc.meshManager = m_meshManager;
	factoryDesc.materialManager = m_materialManager;
	factoryDesc.animationManager = m_animationManager;
	factoryDesc.missingFiles = &missingFiles;

	bool created = false;
	if (m_editor)
	{
		created = SceneFactory::createFromMemory(factoryDesc, fileData, fileSize, &m_scratchAllocator, (Editor::Scene*)m_scene);
	}
	else
	{
		created = SceneFactory::createFromMemory(factoryDesc, fileData, fileSize, &m_scratchAllocator, (Scene*)m_scene);
	}

	auto result = TaskReturnType::Success;
	if (!created)
	{
		std::vector<Event> fileEvents;

		for (auto &it : missingFiles)
		{
			//printf("missing files %u\n", missingFiles.size());
			auto type = it.getType();
			switch (type)
			{
			case vx::FileType::Mesh:
			{
				createTaskLoadMesh(it, &fileEvents);
			}break;
			case vx::FileType::Material:
			{
				createTaskLoadMaterial(it, &fileEvents);
			}break;
			case vx::FileType::Animation:
			{
				createTaskLoadAnimation(it, &fileEvents);
			}break;
			default:
				VX_ASSERT(false);
				break;
			}
		}

		//setTimeoutTime(1000.0f);
		setEventList(&fileEvents);

		result = TaskReturnType::Retry;
	}

	return result;
}