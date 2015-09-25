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
#include <vxEngineLib/FileEntry.h>
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
#include "TaskLoadAudio.h"
#include <vxEngineLib/AudioFile.h>
#include <vxEngineLib/Graphics/Font.h>
#include "TaskLoadFont.h"
#include <vxEngineLib/Actor.h>
#include "TaskLoadActor.h"
#include "TaskSaveActorFile.h"

char ResourceAspect::s_textureFolder[32] = { "data/textures/" };
char ResourceAspect::s_materialFolder[32] = { "data/materials/" };
char ResourceAspect::s_sceneFolder[32] = { "data/scenes/" };
char ResourceAspect::s_meshFolder[32] = { "data/mesh/" };
char ResourceAspect::s_animationFolder[32] = { "data/animation/" };
char ResourceAspect::s_assetFolder[32] = { "../../../assets/" };
char ResourceAspect::s_audioFolder[32] = { "data/audio/" };
char ResourceAspect::s_fontFolder[32] = { "data/fonts/" };
char ResourceAspect::s_actorFolder[32] = { "data/actors/" };

struct ResourceAspect::FileRequest
{
	Event m_event;
	vx::StringID m_sid;
	vx::FileType m_type;
	vx::Variant userData;
	std::string m_filename;
};

struct ResourceAspect::TaskLoadFileDesc
{
	char(&fileNameWithPath)[64];
	const char* fileName;
	vx::StringID sid;
	vx::Variant arg;
	vx::FileType type;

	TaskLoadFileDesc(char(&fileNameWithPath_)[64], const char* filename_, const vx::StringID &sid_, vx::Variant v, vx::FileType type_)
		:fileNameWithPath(fileNameWithPath_), fileName(filename_), sid(sid_), arg(v), type(type_) {}
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
	strcpy_s(s_audioFolder, (dataDir + "audio/").c_str());
	strcpy_s(s_fontFolder, (dataDir + "fonts/").c_str());
	strcpy_s(s_actorFolder, (dataDir + "actors/").c_str());
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
	if (!m_meshResManager.initialize(255, 10 MBYTE, 5 MBYTE, mainAllocator))
		return false;

	if (!m_materialResManager.initialize(255, 0, 5 MBYTE, mainAllocator))
		return false;

	if (!m_animationResManager.initialize(64, 0, 5 MBYTE, mainAllocator))
		return false;

	if (!m_textureResManager.initialize(128, 100 MBYTE, 20 MBYTE, mainAllocator))
		return false;

	if (!m_audioResManager.initialize(128, 10 MBYTE, 1 MBYTE, mainAllocator))
		return false;

	if (!m_fontResManager.initialize(10, 1 MBYTE, 1 MBYTE, mainAllocator))
		return false;

	if (!m_actorResManager.initialize(10, 0 MBYTE, 1 MBYTE, mainAllocator))
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

	m_actorResManager.shutdown();
	m_fontResManager.shutdown();
	m_audioResManager.shutdown();
	m_textureResManager.shutdown();
	m_animationResManager.shutdown();
	m_materialResManager.shutdown();
	m_meshResManager.shutdown();
}

void ResourceAspect::pushFileMessage(vx::FileMessage type, vx::Variant arg1, vx::Variant arg2)
{
	vx::Message msg;
	msg.type = vx::MessageType::File;
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
		arg2 = userData;

		pushFileMessage(vx::FileMessage::Scene_Loaded, arg1, arg2);

		//ResourceAspectCpp::saveMeshesToTxt(m_meshResManager);
	}break;
	case vx::FileType::Mesh:
	{
		vx::Variant arg1;
		arg1.u64 = sid.value;

		vx::Variant arg2;
		arg2 = userData;

		pushFileMessage(vx::FileMessage::Mesh_Loaded, arg1, arg2);
	} break;
	case vx::FileType::Material:
	{
		vx::Variant arg1;
		arg1.u64 = sid.value;

		vx::Variant arg2;
		arg2 = userData;

		pushFileMessage(vx::FileMessage::Material_Loaded, arg1, arg2);
	} break;
	case vx::FileType::Fbx:
	{
		vx::Variant arg1;
		arg1.u64 = sid.value;

		vx::Variant arg2;
		arg2 = userData;

		pushFileMessage(vx::FileMessage::Fbx_Loaded, arg1, arg2);
	} break;
	case vx::FileType::EditorScene:
	{
		vx::Variant arg1;
		arg1.u64 = sid.value;

		vx::Variant arg2;
		arg2 = userData;

		pushFileMessage(vx::FileMessage::EditorScene_Loaded, arg1, arg2);

	}break;
	case vx::FileType::Audio:
	{
		vx::Variant arg1;
		arg1.u64 = sid.value;

		vx::Variant arg2;
		arg2 = userData;

		pushFileMessage(vx::FileMessage::Audio_Loaded, arg1, arg2);
	}break;
	case vx::FileType::Font:
	{
		vx::Variant arg1;
		arg1.u64 = sid.value;

		vx::Variant arg2;
		arg2 = userData;

		pushFileMessage(vx::FileMessage::Font_Loaded, arg1, arg2);
	}break;
	case vx::FileType::Actor:
	{
		vx::Variant arg1;
		arg1.u64 = sid.value;

		vx::Variant arg2;
		arg2 = userData;

		pushFileMessage(vx::FileMessage::Actor_Loaded, arg1, arg2);
	}break;
	default:
		break;
	}
}

void ResourceAspect::update()
{
	m_meshResManager.update();
	m_materialResManager.update();
	m_animationResManager.update();
	m_textureResManager.update();

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

void ResourceAspect::pushFileRequest(vx::FileType fileType, const vx::StringID &sid, const Event &evt, vx::Variant userData, std::string &&filename)
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

void ResourceAspect::pushTask(Task* task, vx::FileType type, const vx::StringID &sid, const Event &evt, vx::Variant p, std::string &&filename)
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

void ResourceAspect::taskLoadFont(const TaskLoadFileDesc &desc, const char* folder, const Event &evt)
{
	auto filename = std::string(desc.fileName);
	taskGetFileNameWithPath(desc, folder);

	TaskLoadFontDesc loadDesc;
	loadDesc.m_fontManager = &m_fontResManager;
	loadDesc.m_textureManager = &m_textureResManager;
	loadDesc.m_filename = filename;
	loadDesc.evt = evt;
	loadDesc.m_fileNameWithPath = std::string(desc.fileNameWithPath);
	loadDesc.m_sid = desc.sid;
	loadDesc.m_path = folder;

	loadDesc.m_resourceAspect = this;
	auto task = new TaskLoadFont(std::move(loadDesc));
	pushTask(task, desc.type, desc.sid, evt, desc.arg, std::move(filename));
}

void ResourceAspect::taskLoadActor(const TaskLoadFileDesc &desc, const char* folder, const Event &evt)
{
	auto filename = std::string(desc.fileName);
	taskGetFileNameWithPath(desc, folder);

	TaskLoadActorDesc loadDesc;
	loadDesc.m_actorResManager = &m_actorResManager;
	loadDesc.m_fileName = filename;
	loadDesc.evt = evt;
	loadDesc.m_fileNameWithPath = std::string(desc.fileNameWithPath);
	loadDesc.m_sid = desc.sid;

	loadDesc.m_resourceAspect = this;
	auto task = new TaskLoadActor(std::move(loadDesc));
	pushTask(task, desc.type, desc.sid, evt, desc.arg, std::move(filename));
}

void ResourceAspect::taskLoadAudio(const TaskLoadFileDesc &desc, const char* folder, const Event &evt)
{
	auto filename = std::string(desc.fileName);
	taskGetFileNameWithPath(desc, folder);

	TaskLoadAudioDesc loadDesc;
	loadDesc.audioDataManager = &m_audioResManager ;
	loadDesc.m_fileName = filename;
	loadDesc.evt = evt;
	loadDesc.m_fileNameWithPath = std::string(desc.fileNameWithPath);
	loadDesc.m_sid = desc.sid;

	auto task = new TaskLoadAudio(std::move(loadDesc));
	pushTask(task, desc.type, desc.sid, evt, desc.arg, std::move(filename));
}

void ResourceAspect::taskLoadScene(const TaskLoadFileDesc &desc, const char* folder, bool editor, const Event &evt)
{
	auto filename = std::string(desc.fileName);
	taskGetFileNameWithPath(desc, folder);

	TaskLoadSceneDirectories directories;
	directories.meshDir = s_meshFolder;
	directories.materialDir = s_materialFolder;
	directories.textureDir = s_textureFolder;
	directories.animDir = s_animationFolder;

	auto scene = desc.arg.ptr;
	TaskLoadSceneDesc loadDesc
	{
		this,
		std::string(desc.fileNameWithPath),
		&m_meshResManager,
		&m_materialResManager,
		&m_animationResManager,
		&m_textureResManager,
		&m_actorResManager,
		scene,
		m_taskManager,
		evt,
		directories,
		m_flipTextures,
		editor
	};

	auto task = new TaskLoadScene(std::move(loadDesc));
	pushTask(task, desc.type, desc.sid, evt, desc.arg, std::move(filename));
}

void ResourceAspect::taskLoadMesh(const TaskLoadFileDesc &desc, const char* folder, const Event &evt)
{
	auto filename = std::string(desc.fileName);
	taskGetFileNameWithPath(desc, folder);

	TaskLoadMeshDesc loadDesc;
	loadDesc.evt = evt;
	loadDesc.m_fileNameWithPath = std::string(desc.fileNameWithPath);
	loadDesc.m_filename = desc.fileName;
	loadDesc.m_meshManager = &m_meshResManager;
	loadDesc.m_sid = desc.sid;

	auto task = new TaskLoadMesh(std::move(loadDesc));
	pushTask(task, desc.type, desc.sid, evt, desc.arg, std::move(filename));
}

void ResourceAspect::taskLoadTexture(const TaskLoadFileDesc &desc, const char* folder, const Event &evt)
{
	auto filename = std::string(desc.fileName);
	taskGetFileNameWithPath(desc, folder);

	TaskLoadTextureDesc loadDesc;
	loadDesc.evt = evt;
	loadDesc.m_fileNameWithPath = std::string(desc.fileNameWithPath);
	loadDesc.m_filename = desc.fileName;
	loadDesc.m_flipImage = 1;
	loadDesc.m_sid = desc.sid;
	loadDesc.m_srgb = desc.arg.u8;
	loadDesc.m_textureManager = &m_textureResManager;

	auto task = new TaskLoadTexture(std::move(loadDesc));
	pushTask(task, desc.type, desc.sid, evt, desc.arg, std::move(filename));
}

void ResourceAspect::taskLoadMaterial(const TaskLoadFileDesc &desc, const char* folder, const Event &evt)
{
	auto filename = std::string(desc.fileName);
	taskGetFileNameWithPath(desc, folder);

	TaskLoadMaterialDesc loadDesc;
	loadDesc.evt = evt;
	loadDesc.m_fileNameWithPath = std::string(desc.fileNameWithPath);
	loadDesc.m_filename = desc.fileName;
	loadDesc.m_flipImage = true;
	loadDesc.m_materialManager = &m_materialResManager;
	loadDesc.m_sid = desc.sid;
	loadDesc.m_taskManager = m_taskManager;
	loadDesc.m_textureFolder = s_textureFolder;
	loadDesc.m_textureManager = &m_textureResManager;

	auto task = new TaskLoadMaterial(std::move(loadDesc));
	pushTask(task, desc.type, desc.sid, evt, desc.arg, std::move(filename));
}

void ResourceAspect::taskLoadFbx(const TaskLoadFileDesc &desc, const char* folder, const Event &evt)
{
	auto filename = std::string(desc.fileName);
	taskGetFileNameWithPath(desc, folder);

	PhsyxMeshType physxMeshType = (PhsyxMeshType)desc.arg.u32;

	TaskLoadFbxDesc loadDesc;
	loadDesc.m_animationFolder = s_animationFolder;
	loadDesc.m_cooking = m_cooking;
	loadDesc.m_event = evt;
	loadDesc.m_fileNameWithPath = std::string(desc.fileNameWithPath);
	loadDesc.m_meshFolder = s_meshFolder;
	loadDesc.m_meshManager = &m_meshResManager;
	loadDesc.m_physxMeshType = physxMeshType;
	loadDesc.m_resourceAspect = this;
	loadDesc.m_userData = desc.arg.ptr;
	loadDesc.m_fbxFactory = m_fbxFactory;

	auto task = new TaskLoadFbx(std::move(loadDesc));
	pushTask(task, desc.type, desc.sid, evt, desc.arg, std::move(filename));
}

void ResourceAspect::taskSaveEditorScene(const TaskLoadFileDesc &desc, const char* folder, const Event &evt)
{
	auto filename = std::string(desc.fileName);
	taskGetFileNameWithPath(desc, folder);

	TaskSaveEditorSceneDesc loadDesc;
	loadDesc.m_evt = evt;
	loadDesc.m_fileNameWithPath = std::string(desc.fileNameWithPath);
	loadDesc.m_scene = (Editor::Scene*)desc.arg.ptr;
	loadDesc.m_actorResManager = &m_actorResManager;

	auto task = new TaskSaveEditorScene(std::move(loadDesc));
	m_taskManager->pushTask(task);
}

void ResourceAspect::taskSaveMeshFile(const TaskLoadFileDesc &desc, const char* folder, const Event &evt)
{
	auto filename = std::string(desc.fileName);
	taskGetFileNameWithPath(desc, folder);

	auto str = std::string(desc.fileNameWithPath);

	auto task = new TaskSaveMeshFile(std::move(str), (vx::MeshFile*)desc.arg.ptr);
	m_taskManager->pushTask(task);
}

void ResourceAspect::taskSaveActor(const TaskLoadFileDesc &desc, const char* folder, const Event &evt)
{
	auto filename = std::string(desc.fileName);
	taskGetFileNameWithPath(desc, folder);

	auto str = std::string(desc.fileNameWithPath);

	auto task = new TaskSaveActorFile(std::move(str), (const ActorFile*)desc.arg.ptr);
	m_taskManager->pushTask(task);
}

void ResourceAspect::requestLoadFile(const vx::FileEntry &fileEntry, vx::Variant arg)
{
	auto evt = Event::createEvent();
	requestLoadFile(fileEntry, arg, evt);
}

void ResourceAspect::requestLoadFile(const vx::FileEntry &fileEntry, vx::Variant arg, const Event &evt)
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
		arg,
		type
	};

	switch (type)
	{
	case vx::FileType::Scene:
	{
		taskLoadScene(desc, s_sceneFolder, false, evt);
	}break;
	case vx::FileType::Invalid:
		break;
	case vx::FileType::Mesh:
	{
		taskLoadMesh(desc, s_meshFolder, evt);
	}break;
	case vx::FileType::Texture:
	{
		taskLoadTexture(desc, s_textureFolder, evt);
	}break;
	case vx::FileType::Material:
	{
		taskLoadMaterial(desc, s_materialFolder, evt);
	}break;
	case vx::FileType::EditorScene:
	{
		taskLoadScene(desc, s_sceneFolder, true, evt);
	}break;
	case vx::FileType::Fbx:
	{
		taskLoadFbx(desc, s_assetFolder, evt);
	}break;
	case vx::FileType::Animation:
	{
		//VX_ASSERT(false);
		//*folder = s_animationFolder;
	}break;
	case vx::FileType::Audio:
	{
		taskLoadAudio(desc, s_audioFolder, evt);
	}break;
	case vx::FileType::Font:
	{
		taskLoadFont(desc, s_fontFolder, evt);
	}break;
	case vx::FileType::Actor:
	{
		taskLoadActor(desc, s_actorFolder, evt);
	}break;
	default:
		break;
	}
}

void ResourceAspect::requestSaveFile(const vx::FileEntry &fileEntry, vx::Variant arg)
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
		arg,
		fileType
	};

	auto evt = Event::createEvent();
	switch (fileType)
	{
	case vx::FileType::EditorScene:
	{
		taskSaveEditorScene(desc, s_sceneFolder, evt);
	}break;
	case vx::FileType::Mesh:
	{
		taskSaveMeshFile(desc, s_meshFolder, evt);
	}break;
	case vx::FileType::Actor:
	{
		taskSaveActor(desc, s_actorFolder, evt);
	}break;
	}
}

const Graphics::Texture* ResourceAspect::getTexture(const vx::StringID &sid) const
{
	return m_textureResManager.find(sid);
}

const Material* ResourceAspect::getMaterial(const vx::StringID &sid) const
{
	return m_materialResManager.find(sid);
}

Material* ResourceAspect::getMaterial(const vx::StringID &sid)
{
	return m_materialResManager.find(sid);
}

const vx::MeshFile* ResourceAspect::getMesh(const vx::StringID &sid) const
{
	return m_meshResManager.find(sid);
}

vx::MeshFile* ResourceAspect::getMesh(const vx::StringID &sid)
{
	return m_meshResManager.find(sid);
}

const vx::Animation* ResourceAspect::getAnimation(const vx::StringID &sid) const
{
	return m_animationResManager.find(sid);
}

const AudioFile* ResourceAspect::getAudioFile(const vx::StringID &sid) const
{
	return m_audioResManager.find(sid);
}

ResourceManager<vx::MeshFile>* ResourceAspect::getMeshManager()
{
	return &m_meshResManager;
}

ResourceManager<Material>* ResourceAspect::getMaterialManager()
{
	return &m_materialResManager;
}

const Graphics::Font* ResourceAspect::getFontFile(const vx::StringID &sid) const
{
	return m_fontResManager.find(sid);
}

const Actor* ResourceAspect::getActor(const vx::StringID &sid) const
{
	return m_actorResManager.find(sid);
}

Actor* ResourceAspect::addActor(const vx::StringID &sid, std::string &&name, Actor &actor)
{
	return m_actorResManager.insertEntry(sid,std::move(name), actor);
}