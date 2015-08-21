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
#include "TaskLoadMesh.h"
#include "TaskLoadTexture.h"
#include "TaskLoadMaterial.h"
#include "TaskLoadFbx.h"
#include "TaskSaveEditorScene.h"
#include <Windows.h>
#include <FbxFactoryInterface.h>
#include "TaskSaveMeshFile.h"

char ResourceAspect::s_textureFolder[32] = { "data/textures/" };
char ResourceAspect::s_materialFolder[32] = { "data/materials/" };
char ResourceAspect::s_sceneFolder[32] = { "data/scenes/" };
char ResourceAspect::s_meshFolder[32] = { "data/mesh/" };
char ResourceAspect::s_animationFolder[32] = { "data/animation/" };
char ResourceAspect::s_assetFolder[32] = { "../../../assets/" };

struct ResourceAspect::FileRequest
{
	Event m_event;
	vx::StringID m_sid;
	vx::FileType m_type;
	void* userData;
	std::string m_filename;
};

struct ResourceAspect::TaskLoadFileDesc
{
	char(&fileNameWithPath)[64];
	const char* fileName;
	vx::StringID sid;
	void* p;
	vx::FileType type;

	TaskLoadFileDesc(char(&fileNameWithPath_)[64], const char* filename_, const vx::StringID &sid_, void* ptr, vx::FileType type_)
		:fileNameWithPath(fileNameWithPath_), fileName(filename_), sid(sid_), p(ptr), type(type_) {}
};

ResourceAspect::ResourceAspect()
	:m_fbxFactory(nullptr),
	m_dllHandle(nullptr)
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
	strcpy_s(s_assetFolder, (dataDir + "../../assets/").c_str());
}

bool ResourceAspect::loadFbxFactory()
{
#if _DEBUG
	auto handle = LoadLibrary(L"../../../lib/vxFbxImporter_d.dll");
#else
	auto handle = LoadLibrary(L"../../../lib/vxFbxImporter.dll");
#endif

	if (handle == nullptr)
		return false;

	CreateFbxFactoryFunctionType proc = (CreateFbxFactoryFunctionType)GetProcAddress(handle, "createFbxFactory");
	if (proc == nullptr)
		return false;

	m_fbxFactory = proc();
	m_dllHandle = handle;

	return true;
}

bool ResourceAspect::initialize(vx::StackAllocator *mainAllocator, const std::string &dataDir, physx::PxCooking* cooking, vx::TaskManager* taskManager, vx::MessageManager* msgManager, bool flipTextures, bool editor)
{
	if (!m_meshData.initialize(255, 10 MBYTE, 5 MBYTE, mainAllocator))
		return false;

	if (!m_materialData.initialize(255, 0, 5 MBYTE, mainAllocator))
		return false;

	if (!m_animationData.initialize(64, 0, 5 MBYTE, mainAllocator))
		return false;

	if (!m_textureData.initialize(128, 50 MBYTE, 20 MBYTE, mainAllocator))
		return false;

	if (editor)
	{
		if (!loadFbxFactory())
			return false;
	}

	m_taskManager = taskManager;
	m_msgManager = msgManager;

	setDirectories(dataDir);

	m_flipTextures = flipTextures;
	m_cooking = cooking;

	return true;
}

void ResourceAspect::shutdown()
{
	if (m_dllHandle)
	{
		auto proc = (DestroyFbxFactoryFunctionType)GetProcAddress((HMODULE)m_dllHandle, "destroyFbxFactory");
		proc(m_fbxFactory);
		m_fbxFactory = nullptr;

		FreeLibrary((HMODULE)m_dllHandle);
		m_dllHandle = nullptr;
	}

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

		//ResourceAspectCpp::saveMeshesToTxt(m_meshData);
	}break;
	case vx::FileType::Mesh:
	{
		vx::Variant arg1;
		arg1.u64 = sid.value;

		vx::Variant arg2;
		arg2.ptr = userData;

		pushFileMessage(vx::FileMessage::Mesh_Loaded, arg1, arg2);
	} break;
	case vx::FileType::Material:
	{
		vx::Variant arg1;
		arg1.u64 = sid.value;

		vx::Variant arg2;
		arg2.ptr = userData;

		pushFileMessage(vx::FileMessage::Material_Loaded, arg1, arg2);
	} break;
	case vx::FileType::Fbx:
	{
		vx::Variant arg1;
		arg1.u64 = sid.value;

		vx::Variant arg2;
		arg2.ptr = userData;

		pushFileMessage(vx::FileMessage::Fbx_Loaded, arg1, arg2);
	} break;
	case vx::FileType::EditorScene:
	{
		vx::Variant arg1;
		arg1.u64 = sid.value;

		vx::Variant arg2;
		arg2.ptr = userData;

		pushFileMessage(vx::FileMessage::EditorScene_Loaded, arg1, arg2);

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

	std::lock_guard<std::mutex> lock(m_requestMutex);

	std::vector<FileRequest> requests;
	requests.reserve(m_requests.size());
	for (auto &it : m_requests)
	{
		auto evtStatus = it.m_event.getStatus();
		switch (evtStatus)
		{
		case EventStatus::Error:
			VX_ASSERT(false);
			break;
		case EventStatus::Complete:
		{
			sendFileMessage(it);
		}break;
		default:
			requests.push_back(it);
			break;
		}
	}

	m_requests.swap(requests);
}

void ResourceAspect::pushFileRequest(vx::FileType fileType, const vx::StringID &sid, const Event &evt, void* userData, std::string &&filename)
{
	std::lock_guard<std::mutex> lock(m_requestMutex);

	FileRequest request;
	request.m_event = evt;
	request.m_sid = sid;
	request.m_type = fileType;
	request.userData = userData;
	request.m_filename = std::move(filename);
	m_requests.push_back(request);
}

void ResourceAspect::pushTask(Task* task, vx::FileType type, const vx::StringID &sid, const Event &evt, void* p, std::string &&filename)
{
	m_taskManager->pushTask(task);

	pushFileRequest(type, sid, evt, p, std::move(filename));
}

void ResourceAspect::taskGetFileNameWithPath(const TaskLoadFileDesc &desc, const char* folder)
{
	auto returnCode = sprintf_s(desc.fileNameWithPath, "%s%s", folder, desc.fileName);
	if (returnCode == -1)
	{
		VX_ASSERT(false);
	}
}

void ResourceAspect::taskLoadScene(const TaskLoadFileDesc &desc, const char* folder, bool editor)
{
	auto filename = std::string(desc.fileName);
	taskGetFileNameWithPath(desc, folder);

	TaskLoadSceneDirectories directories;
	directories.meshDir = s_meshFolder;
	directories.materialDir = s_materialFolder;
	directories.textureDir = s_textureFolder;
	directories.animDir = s_animationFolder;

	auto evt = Event::createEvent();
	auto scene = desc.p;
	TaskLoadSceneDesc loadDesc
	{
		std::string(desc.fileNameWithPath),
		&m_meshData,
		&m_materialData,
		&m_animationData,
		&m_textureData,
		scene,
		m_taskManager,
		evt,
		directories,
		m_flipTextures,
		editor
	};

	auto task = new TaskLoadScene(std::move(loadDesc));
	pushTask(task, desc.type, desc.sid, evt, desc.p, std::move(filename));
}

void ResourceAspect::taskLoadMesh(const TaskLoadFileDesc &desc, const char* folder)
{
	auto filename = std::string(desc.fileName);
	taskGetFileNameWithPath(desc, folder);

	auto evt = Event::createEvent();

	TaskLoadMeshDesc loadDesc;
	loadDesc.evt = evt;
	loadDesc.m_fileNameWithPath = std::string(desc.fileNameWithPath);
	loadDesc.m_filename = desc.fileName;
	loadDesc.m_meshManager = &m_meshData;
	loadDesc.m_sid = desc.sid;

	auto task = new TaskLoadMesh(std::move(loadDesc));
	pushTask(task, desc.type, desc.sid, evt, desc.p, std::move(filename));
}

void ResourceAspect::taskLoadTexture(const TaskLoadFileDesc &desc, const char* folder)
{
	auto filename = std::string(desc.fileName);
	taskGetFileNameWithPath(desc, folder);

	auto evt = Event::createEvent();

	TaskLoadTextureDesc loadDesc;
	loadDesc.evt = evt;
	loadDesc.m_fileNameWithPath = std::string(desc.fileNameWithPath);
	loadDesc.m_filename = desc.fileName;
	loadDesc.m_flipImage = true;
	loadDesc.m_sid = desc.sid;
	// need srgb flag
	VX_ASSERT(false);
	loadDesc.m_srgb = false;
	loadDesc.m_textureManager = &m_textureData;

	auto task = new TaskLoadTexture(std::move(loadDesc));
	pushTask(task, desc.type, desc.sid, evt, desc.p, std::move(filename));
}

void ResourceAspect::taskLoadMaterial(const TaskLoadFileDesc &desc, const char* folder)
{
	auto filename = std::string(desc.fileName);
	taskGetFileNameWithPath(desc, folder);

	auto evt = Event::createEvent();

	TaskLoadMaterialDesc loadDesc;
	loadDesc.evt = evt;
	loadDesc.m_fileNameWithPath = std::string(desc.fileNameWithPath);
	loadDesc.m_filename = desc.fileName;
	loadDesc.m_flipImage = true;
	loadDesc.m_materialManager = &m_materialData;
	loadDesc.m_sid = desc.sid;
	loadDesc.m_taskManager = m_taskManager;
	loadDesc.m_textureFolder = s_textureFolder;
	loadDesc.m_textureManager = &m_textureData;

	auto task = new TaskLoadMaterial(std::move(loadDesc));
	pushTask(task, desc.type, desc.sid, evt, desc.p, std::move(filename));
}

void ResourceAspect::taskLoadFbx(const TaskLoadFileDesc &desc, const char* folder)
{
	auto filename = std::string(desc.fileName);
	taskGetFileNameWithPath(desc, folder);

	PhsyxMeshType physxMeshType = (PhsyxMeshType)reinterpret_cast<u32>(desc.p);

	auto evt = Event::createEvent();

	TaskLoadFbxDesc loadDesc;
	loadDesc.m_animationFolder = s_animationFolder;
	loadDesc.m_cooking = m_cooking;
	loadDesc.m_event = evt;
	loadDesc.m_fileNameWithPath = std::string(desc.fileNameWithPath);
	loadDesc.m_meshFolder = s_meshFolder;
	loadDesc.m_meshManager = &m_meshData;
	loadDesc.m_physxMeshType = physxMeshType;
	loadDesc.m_resourceAspect = this;
	loadDesc.m_userData = desc.p;
	loadDesc.m_fbxFactory = m_fbxFactory;

	auto task = new TaskLoadFbx(std::move(loadDesc));
	pushTask(task, desc.type, desc.sid, evt, desc.p, std::move(filename));
}

void ResourceAspect::taskSaveEditorScene(const TaskLoadFileDesc &desc, const char* folder)
{
	auto filename = std::string(desc.fileName);
	taskGetFileNameWithPath(desc, folder);
	auto evt = Event::createEvent();

	TaskSaveEditorSceneDesc loadDesc;
	loadDesc.m_evt = evt;
	loadDesc.m_fileNameWithPath = std::string(desc.fileNameWithPath);
	loadDesc.m_scene = (Editor::Scene*)desc.p;

	auto task = new TaskSaveEditorScene(std::move(loadDesc));
	m_taskManager->pushTask(task);
}

void ResourceAspect::taskSaveMeshFile(const TaskLoadFileDesc &desc, const char* folder)
{
	auto filename = std::string(desc.fileName);
	taskGetFileNameWithPath(desc, folder);
	auto evt = Event::createEvent();

	auto str = std::string(desc.fileNameWithPath);

	auto task = new TaskSaveMeshFile(std::move(str), (vx::MeshFile*)desc.p);
	m_taskManager->pushTask(task);
}

void ResourceAspect::requestLoadFile(const vx::FileEntry &fileEntry, vx::Variant arg)
{
	char fileNameWithPath[64];

	auto type = fileEntry.getType();
	auto sid = fileEntry.getSid();
	auto fileName = fileEntry.getString();

	TaskLoadFileDesc desc
	{
		fileNameWithPath,
		fileName,
		sid,
		arg.ptr,
		type
	};

	switch (type)
	{
	case vx::FileType::Scene:
	{
		taskLoadScene(desc, s_sceneFolder, false);
	}break;
	case vx::FileType::Invalid:
		break;
	case vx::FileType::Mesh:
	{
		taskLoadMesh(desc, s_meshFolder);
	}break;
	case vx::FileType::Texture:
	{
		VX_ASSERT(false);
		//*folder = s_textureFolder;
	}break;
	case vx::FileType::Material:
	{
		taskLoadMaterial(desc, s_materialFolder);
	}break;
	case vx::FileType::EditorScene:
	{
		taskLoadScene(desc, s_sceneFolder, true);
	}break;
	case vx::FileType::Fbx:
	{
		taskLoadFbx(desc, s_assetFolder);
	}break;
	case vx::FileType::Animation:
	{
		//VX_ASSERT(false);
		//*folder = s_animationFolder;
	}break;
	default:
		break;
	}
}

void ResourceAspect::requestSaveFile(const vx::FileEntry &fileEntry, void* p)
{
	auto fileType = fileEntry.getType();
	const char* fileName = fileEntry.getString();
	auto sid = fileEntry.getSid();

	char fileNameWithPath[64];
	TaskLoadFileDesc desc
	{
		fileNameWithPath,
		fileName,
		sid,
		p,
		fileType
	};

	switch (fileType)
	{
	case vx::FileType::EditorScene:
	{
		taskSaveEditorScene(desc, s_sceneFolder);
	}break;
	case vx::FileType::Mesh:
	{
		taskSaveMeshFile(desc, s_meshFolder);
	}break;
	}
}

const Graphics::Texture* ResourceAspect::getTexture(const vx::StringID &sid) const
{
	return m_textureData.find(sid);
}

const Material* ResourceAspect::getMaterial(const vx::StringID &sid) const
{
	return m_materialData.find(sid);
}

Material* ResourceAspect::getMaterial(const vx::StringID &sid)
{
	return m_materialData.find(sid);
}

const vx::MeshFile* ResourceAspect::getMesh(const vx::StringID &sid) const
{
	return m_meshData.find(sid);
}

vx::MeshFile* ResourceAspect::getMesh(const vx::StringID &sid)
{
	return m_meshData.find(sid);
}

const vx::Animation* ResourceAspect::getAnimation(const vx::StringID &sid) const
{
	return m_animationData.find(sid);
}

ResourceManager<vx::MeshFile>* ResourceAspect::getMeshManager()
{
	return &m_meshData;
}