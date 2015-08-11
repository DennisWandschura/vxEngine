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
#include <vxResourceAspect/ResourceAspect.h>
#include <vxEngineLib/Animation.h>
#include <vxEngineLib/Material.h>
#include <vxEngineLib/MeshFile.h>
#include <vxEngineLib/Graphics/Texture.h>
#include <vxEngineLib/Event.h>
#include <vxResourceAspect/FileEntry.h>
#include "TaskLoadScene.h"
#include <vxEngineLib/TaskManager.h>
#include <vxEngineLib/FileMessage.h>
#include <vxEngineLib/Message.h>
#include <vxEngineLib/MessageTypes.h>
#include <vxEngineLib/MessageManager.h>

char ResourceAspect::s_textureFolder[32] = { "data/textures/" };
char ResourceAspect::s_materialFolder[32] = { "data/materials/" };
char ResourceAspect::s_sceneFolder[32] = { "data/scenes/" };
char ResourceAspect::s_meshFolder[32] = { "data/mesh/" };
char ResourceAspect::s_animationFolder[32] = { "data/animation/" };

struct ResourceAspect::FileRequest
{
	shared_ptr<Event> m_event;
	vx::StringID m_sid;
	vx::FileType m_type;
	void* userData;
};

ResourceAspect::ResourceAspect()
{

}

ResourceAspect::~ResourceAspect()
{

}

void ResourceAspect::setDirectories(const std::string &dataDir)
{
	strcpy_s(s_textureFolder, (dataDir + "textures/").c_str());
	strcpy_s(s_materialFolder, (dataDir + "materials/").c_str());
	strcpy_s(s_sceneFolder, (dataDir + "scenes/").c_str());
	strcpy_s(s_meshFolder, (dataDir + "mesh/").c_str());
	strcpy_s(s_animationFolder, (dataDir + "animation/").c_str());

	//strcpy_s(FileAspectCpp::g_assetFolder, (dataDir + "../../assets/").c_str());
}

bool ResourceAspect::initialize(vx::StackAllocator *mainAllocator, const std::string &dataDir, vx::TaskManager* taskManager, vx::MessageManager* msgManager)
{
	if (!m_meshData.initialize(255, 10 MBYTE, 5 MBYTE, mainAllocator))
		return false;

	if (!m_materialData.initialize(255, 0, 5 MBYTE, mainAllocator))
		return false;

	if (!m_animationData.initialize(64, 0, 5 MBYTE, mainAllocator))
		return false;

	if (!m_textureData.initialize(128, 50 MBYTE, 15 MBYTE, mainAllocator))
		return false;

	m_taskManager = taskManager;
	m_msgManager = msgManager;

	setDirectories(dataDir);

	return true;
}

void ResourceAspect::shutdown()
{
	m_textureData.shutdown();
	m_animationData.shutdown();
	m_materialData.shutdown();
	m_meshData.shutdown();
}

void ResourceAspect::pushFileMessage(vx::FileMessage type, vx::Variant arg1, vx::Variant arg2)
{
	vx::Message msg;
	msg.type = vx::MessageType::File_Event;
	msg.code = (u32)type;
	msg.arg1 = arg1;
	msg.arg2 = arg2;

	m_msgManager->addMessage(msg);
}

void ResourceAspect::sendFileMessage(const FileRequest &request)
{
	auto sid = request.m_sid;
	auto type = request.m_type;
	auto userData = request.userData;

	switch (type)
	{
	case vx::FileType::Scene:
	{
		vx::Variant arg1;
		arg1.u64 = sid.value;

		vx::Variant arg2;
		arg2.ptr = userData;

		pushFileMessage(vx::FileMessage::Scene_Loaded, arg1, arg2);
	}break;
	default:
		break;
	}
}

void ResourceAspect::update()
{
	m_meshData.update();
	m_materialData.update();
	m_animationData.update();
	m_textureData.update();

	std::vector<FileRequest> requests;
	requests.reserve(m_requests.size());
	for (auto &it : m_requests)
	{
		auto evtStatus = it.m_event->getStatus();
		switch (evtStatus)
		{
		case EventStatus::Error:
			VX_ASSERT(false);
			break;
		case EventStatus::Complete:
		{
			sendFileMessage(it);
			m_meshData.clearScratchAllocator();
			m_animationData.clearScratchAllocator();
			m_materialData.clearScratchAllocator();
			m_textureData.clearScratchAllocator();
		}break;
		default:
			requests.push_back(it);
			break;
		}
	}

	m_requests.swap(requests);
}

void ResourceAspect::pushFileRequest(vx::FileType fileType, const vx::StringID &sid, const shared_ptr<Event> &evt, void* userData)
{
	FileRequest request;
	request.m_event = evt;
	request.m_sid = sid;
	request.m_type = fileType;
	request.userData = userData;
	m_requests.push_back(request);
}

void ResourceAspect::requestLoadFile(const vx::FileEntry &fileEntry, void* p)
{
	char fileNameWithPath[64];

	auto type = fileEntry.getType();
	auto sid = fileEntry.getSid();
	auto fileName = fileEntry.getString();

	switch (type)
	{
	case vx::FileType::Scene:
	{
		//*folder = s_sceneFolder;

		const char* folder = s_sceneFolder;

		auto returnCode = sprintf_s(fileNameWithPath, "%s%s", folder, fileName);
		if (returnCode == -1)
		{
			VX_ASSERT(false);
		}

		TaskLoadSceneDirectories directories;
		directories.meshDir = s_meshFolder;
		directories.materialDir = s_materialFolder;
		directories.textureDir = s_textureFolder;
		directories.animDir = s_animationFolder;

		auto evt = shared_ptr<Event>(new Event());
		auto scene = (Scene*)p;
		TaskLoadSceneDesc desc
		{
			std::string(fileNameWithPath),
			&m_meshData,
			&m_materialData,
			&m_animationData,
			&m_textureData,
			scene,
			m_taskManager,
			evt,
			directories
		};

		auto task = new TaskLoadScene(std::move(desc));
		m_taskManager->pushTask(task);

		pushFileRequest(type, sid, evt, p);
	}break;
	case vx::FileType::Invalid:
		break;
	case vx::FileType::Mesh:
	{
		//*folder = s_meshFolder;
	}break;
	case vx::FileType::Texture:
	{
		//*folder = s_textureFolder;
	}break;
	case vx::FileType::Material:
	{
		//*folder = s_materialFolder;
	}break;
	case vx::FileType::EditorScene:
	{
		//*folder = s_sceneFolder;
	}break;
	case vx::FileType::Fbx:
	{
		// *folder = FileAspectCpp::g_assetFolder;
	}break;
	case vx::FileType::Animation:
	{
		//*folder = s_animationFolder;
	}break;
	default:
		break;
	}
}

void ResourceAspect::requestSaveFile(const vx::FileEntry &fileEntry, void* p)
{

}

Reference<Graphics::Texture> ResourceAspect::getTexture(const vx::StringID &sid) const
{
	return m_textureData.find(sid);
}

Reference<Material> ResourceAspect::getMaterial(const vx::StringID &sid) const
{
	return m_materialData.find(sid);
}

Reference<vx::MeshFile> ResourceAspect::getMesh(const vx::StringID &sid) const
{
	return m_meshData.find(sid);
}

Reference<vx::Animation> ResourceAspect::getAnimation(const vx::StringID &sid) const
{
	return m_animationData.find(sid);
}