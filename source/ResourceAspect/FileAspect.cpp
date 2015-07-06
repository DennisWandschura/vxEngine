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
#include <vxResourceAspect/FileAspect.h>
#include <vxResourceAspect/MaterialFactory.h>
#include <vxResourceAspect/SceneFactory.h>
#include <vxResourceAspect/FileFactory.h>
#include <vxResourceAspect/CreateSceneDescription.h>
#include <vxLib/File/File.h>
#include <vxLib/ScopeGuard.h>
#include <vxEngineLib/Light.h>
#include <vxEngineLib/EventManager.h>
#include <vxEngineLib/Event.h>
#include <vxEngineLib/EventTypes.h>
#include <vxLib/File/FileHeader.h>
#include <vxEngineLib/debugPrint.h>
#include <vxEngineLib/AnimationFile.h>
#include <vxEngineLib/MeshFile.h>
#include <vxEngineLib/Material.h>
#include <vxEngineLib/FileEvents.h>
#include <vxResourceAspect/FbxFactory.h>
#include <vxEngineLib/AnimationFile.h>
#include <vxEngineLib/FileFactory.h>

char FileAspect::s_textureFolder[32] = { "data/textures/" };
char FileAspect::s_materialFolder[32] = { "data/materials/" };
char FileAspect::s_sceneFolder[32] = { "data/scenes/" };
char FileAspect::s_meshFolder[32] = { "data/mesh/" };
char FileAspect::s_animationFolder[32] = { "data/animation/" };

namespace FileAspectCpp
{
	char g_assetFolder[32] = {"../../../assets/"};
}

namespace
{
	template<typename T>
	void createPool(u16 maxCount, vx::Pool<T> *pool, vx::StackAllocator* allocator)
	{
		auto sizeInBytes = sizeof(T) * maxCount;

		pool->initialize(allocator->allocate(sizeInBytes, 64), maxCount);
	}

	template<typename T>
	void destroyPool(vx::Pool<T> *pool)
	{
		pool->clear();
		pool->release();
	}
}

struct FileAspect::FileRequest
{
	enum OpenType : u8{ Load = 0, Save = 1 };

	vx::FileEntry m_fileEntry;
	void* userData{ nullptr };
	u8 m_maxRetries{ 10 };
	OpenType m_openType{};
};

struct FileAspect::LoadFileOfTypeDescription
{
	const char* fileName;
	vx::StringID sid;
	LoadFileReturnType* result;
	void* pUserData;
	vx::FileType fileType;
	const char* fileNameWithPath;
	u32 fileSize;
	const u8* fileData;
	std::vector<vx::FileEntry>* missingFiles;
};

struct FileAspect::LoadDescriptionShared
{
	const char *filename;
	vx::StringID sid;
	vx::FileStatus* status;
};

struct FileAspect::LoadMeshDescription
{
	LoadDescriptionShared shared;
	const vx::FileHeader* fileHeader;
	const u8 *fileData;
	u32 size;
};

struct FileAspect::LoadMaterialDescription
{
	LoadDescriptionShared shared;
	const char *fileNameWithPath; 
	std::vector<vx::FileEntry>* missingFiles; 
};

FileAspect::FileAspect()
	:m_fileRequests(),
	m_logfile(m_timer),
	m_allocatorReadFile(),
	m_timer(),
	m_poolMesh(),
	m_poolMaterial(),
	m_textureFileManager(),
	m_eventManager(nullptr),
	m_cooking(nullptr)
{
}

FileAspect::~FileAspect()
{
}

bool FileAspect::initialize(vx::StackAllocator *pMainAllocator, const std::string &dataDir, vx::EventManager* evtManager, physx::PxCooking* cooking)
{
	m_eventManager = evtManager;
	const auto fileMemorySize = 5 MBYTE;
	auto pFileMemory = pMainAllocator->allocate(fileMemorySize, 64);
	if (!pFileMemory)
		return false;

	m_scratchAllocator = vx::StackAllocator(pMainAllocator->allocate(fileMemorySize, 16), fileMemorySize);
	m_allocatorReadFile = vx::StackAllocator(pFileMemory, fileMemorySize);

	const u32 textureMemorySize = 10 MBYTE;
	m_allocatorMeshData = vx::StackAllocator(pMainAllocator->allocate(textureMemorySize, 64), textureMemorySize);

	m_logfile.create("filelog.xml");

	const u32 maxCount = 255;
	m_sortedMeshes = vx::sorted_array<vx::StringID, vx::MeshFile*>(maxCount, pMainAllocator);
	m_sortedMaterials = vx::sorted_array<vx::StringID, Reference<Material>>(maxCount, pMainAllocator);

	createPool(maxCount, &m_poolMesh, pMainAllocator);
	createPool(maxCount, &m_poolMaterial, pMainAllocator);
	createPool(maxCount, &m_poolAnimations, pMainAllocator);
	m_textureFileManager.initialize(maxCount, pMainAllocator);

	m_cooking = cooking;

	strcpy_s(s_textureFolder, (dataDir + "textures/").c_str());
	strcpy_s(s_materialFolder, (dataDir + "materials/").c_str());
	strcpy_s(s_sceneFolder, (dataDir + "scenes/").c_str());
	strcpy_s(s_meshFolder, (dataDir + "mesh/").c_str());
	strcpy_s(s_animationFolder, (dataDir + "animation/").c_str());

	strcpy_s(FileAspectCpp::g_assetFolder, (dataDir + "../../assets/").c_str());
	

	return true;
}

void FileAspect::shutdown()
{
	m_textureFileManager.shutdown();

	m_sortedMaterials.cleanup();
	m_sortedMeshes.cleanup();

	destroyPool(&m_poolMaterial);
	destroyPool(&m_poolMesh);

	m_logfile.close();
	m_allocatorReadFile.release();
}

bool FileAspect::loadMesh(const LoadMeshDescription &desc)
{
	vx::lock_guard<vx::SRWMutex> lock(m_mutexLoadedFiles);

	bool result = true;
	auto it = m_sortedMeshes.find(desc.shared.sid);
	if (it == m_sortedMeshes.end())
	{
		u16 index;
		auto meshFilePtr = m_poolMesh.createEntry(&index, desc.fileHeader->version);
		VX_ASSERT(meshFilePtr != nullptr);

		auto marker = m_allocatorMeshData.getMarker();
		auto p = meshFilePtr->loadFromMemory(desc.fileData, desc.size, &m_allocatorMeshData);
		if (p == nullptr)
		{
			m_allocatorMeshData.clear(marker);
			return false;
		}

		it = m_sortedMeshes.insert(desc.shared.sid, meshFilePtr);

		*desc.shared.status = vx::FileStatus::Loaded;

		LOG_ARGS(m_logfile, "Loaded Mesh '%s' %llu\n", false, desc.shared.filename, desc.shared.sid.value);
	}
	else
	{
		*desc.shared.status = vx::FileStatus::Exists;
	}

	return result;
}

bool FileAspect::loadFileScene(const LoadFileOfTypeDescription &desc, bool editor)
{
	bool result = false;
	{
		Factory::CreateSceneDescription factoryDesc;
		factoryDesc.loadedFiles = &m_loadedFiles;
		factoryDesc.materials = &m_sortedMaterials;
		factoryDesc.meshes = &m_sortedMeshes;
		factoryDesc.pMissingFiles = desc.missingFiles;

		bool created = false;
		if (editor)
		{
			created = SceneFactory::createFromMemory(factoryDesc, desc.fileData, desc.fileSize, &m_scratchAllocator, (Editor::Scene*)desc.pUserData);
		}
		else
		{
			created = SceneFactory::createFromMemory(factoryDesc, desc.fileData, desc.fileSize, &m_scratchAllocator, (Scene*)desc.pUserData);
		}

		if (created)
		{
			desc.result->status = vx::FileStatus::Loaded;
			LOG_ARGS(m_logfile, "Loaded scene '%s' %llu\n", false, desc.fileName, desc.sid.value);
			result = true;
		}
		else
		{
			printf("Error loading scene '%s'\n", desc.fileName);
			LOG_ERROR_ARGS(m_logfile, "Error loading scene '%s'\n", false, desc.fileName);
		}
	}
	return result;
}

Reference<Material> FileAspect::loadMaterial(const LoadMaterialDescription &desc)
{
	Reference<Material> result;

	vx::lock_guard<vx::SRWMutex> lock(m_mutexLoadedFiles);

	auto it = m_sortedMaterials.find(desc.shared.sid);
	if (it == m_sortedMaterials.end())
	{
		Material material(desc.shared.sid);

		MaterialFactoryLoadDescription factoryDesc;
		factoryDesc.fileNameWithPath = desc.fileNameWithPath;
		factoryDesc.textureFiles = &m_textureFileManager.getSortedTextureFiles();
		factoryDesc.missingFiles = desc.missingFiles;
		factoryDesc.material = &material;
		auto result = MaterialFactory::load(factoryDesc);

		if (result)
		{
			u16 index;
			auto materialPtr = m_poolMaterial.createEntry(&index, std::move(material));
			VX_ASSERT(materialPtr != nullptr);

			m_sortedMaterials.insert(desc.shared.sid, Reference<Material>(*materialPtr));

			result = materialPtr;
			*desc.shared.status = vx::FileStatus::Loaded;
			LOG_ARGS(m_logfile, "Loaded Material '%s'\n", false, desc.shared.filename);
		}
		else
		{
			LOG_WARNING_ARGS(m_logfile, "Error loading Material '%s'\n", false, desc.shared.filename);
		}
	}
	else
	{
		*desc.shared.status = vx::FileStatus::Exists;
		result = (*it);
	}

	return result;
}

void FileAspect::getFolderString(vx::FileType fileType, const char** folder)
{
	switch (fileType)
	{
	case vx::FileType::Texture:
		*folder = s_textureFolder;
		break;
	case vx::FileType::Mesh:
		*folder = s_meshFolder;
		break;
	case vx::FileType::Material:
		*folder = s_materialFolder;
		break;
	case vx::FileType::Scene:
		*folder = s_sceneFolder;
		break;
	case vx::FileType::EditorScene:
			*folder = s_sceneFolder;
		break;
	case vx::FileType::Fbx:
		*folder = FileAspectCpp::g_assetFolder;
		break;
	case vx::FileType::Animation:
		*folder = s_animationFolder;
		break;
	default:
		break;
	}
}

const u8* FileAspect::readFile(const char *file, u32* fileSize)
{
	vx::File f;
	if (!f.open(file, vx::FileAccess::Read))
	{
		LOG_ERROR_ARGS(m_logfile, "Error opening file '%s'\n", false, file);
		return nullptr;
	}

	*fileSize = f.getSize();

	VX_ASSERT(*fileSize != 0);

	auto pData = (u8*)m_allocatorReadFile.allocate(*fileSize, 8);
	VX_ASSERT(pData);

	if (!f.read(pData, *fileSize))
	{
		LOG_ERROR_ARGS(m_logfile, "Error reading file '%s'\n", false, file);
		return nullptr;
	}

	return pData;
}

void FileAspect::pushFileEvent(vx::FileEvent code, vx::Variant arg1, vx::Variant arg2)
{
	vx::Event e;
	e.arg1 = arg1;
	e.arg2 = arg2;
	e.type = vx::EventType::File_Event;
	e.code = (u32)code;

	m_eventManager->addEvent(e);
}

bool FileAspect::loadFileAnimation(const LoadFileOfTypeDescription &desc)
{
	vx::FileHeader headerTop;
	memcpy(&headerTop, desc.fileData, sizeof(vx::FileHeader));

	if (headerTop.s_magic != headerTop.magic)
		return false;

	auto dataBegin = desc.fileData + sizeof(vx::FileHeader);
	auto fileDataEnd = desc.fileData + desc.fileSize;

	vx::FileHeader headerBottom;
	memcpy(&headerBottom, fileDataEnd - sizeof(vx::FileHeader), sizeof(vx::FileHeader));

	if (!headerBottom.isEqual(headerTop))
	{
		printf("invalid file\n");
		return false;
	}

	auto size = desc.fileSize;

	vx::AnimationFile animFile(vx::AnimationFile::getGlobalVersion());

	animFile.loadFromMemory(dataBegin, size, nullptr);
	auto crc = animFile.getCrc();

	if (headerTop.crc != crc)
	{
		printf("wrong crc: %llu %llu\n", headerTop.crc, crc);
		return false;
	}

	u16 index;
	auto ptr = m_poolAnimations.createEntry(&index, std::move(animFile));
	if (ptr == nullptr)
	{
		return false;
	}

	vx::verboseChannelPrintF(0, vx::debugPrint::Channel_FileAspect, "Loaded Animation %s\n", desc.fileName);
	m_sortedAnimations.insert(desc.sid, ptr);

	return true;
}

bool FileAspect::loadFileFbx(const LoadFileOfTypeDescription &desc)
{
	vx::verboseChannelPrintF(0, vx::debugPrint::Channel_FileAspect, "Trying to load file %s\n", desc.fileNameWithPath);

	std::vector<vx::FileHandle> meshFiles, animFiles;
	FbxFactory factory;
	factory.loadFile(desc.fileNameWithPath, std::string(s_meshFolder), std::string(s_animationFolder), m_cooking, &meshFiles, &animFiles);

	for (auto &it : meshFiles)
	{
		vx::FileEntry fileEntry(it.m_string, vx::FileType::Mesh);
		requestLoadFile(fileEntry, desc.pUserData);
	}

	for (auto &it : animFiles)
	{
		vx::FileEntry fileEntry(it.m_string, vx::FileType::Animation);
		requestLoadFile(fileEntry, desc.pUserData);
	}
	return true;
}

bool FileAspect::loadFileMesh(const LoadFileOfTypeDescription &desc)
{
	vx::FileHeader header;
	memcpy(&header, desc.fileData, sizeof(vx::FileHeader));
	auto meshFileDataBegin = desc.fileData + sizeof(vx::FileHeader);

	if (header.magic != header.s_magic)
	{
		desc.result->result = 0;
		vx::verboseChannelPrintF(0, vx::debugPrint::Channel_FileAspect, "Error loading mesh '%s'", desc.fileName);
		return false;
	}

	LoadMeshDescription loadMeshDesc;
	loadMeshDesc.fileHeader = &header;
	loadMeshDesc.shared.filename = desc.fileName;
	loadMeshDesc.size = desc.fileSize;
	loadMeshDesc.fileData = meshFileDataBegin;
	loadMeshDesc.shared.sid = desc.sid;
	loadMeshDesc.shared.status = &desc.result->status;

	bool result = false;
	if (loadMesh(loadMeshDesc))
	{
		desc.result->result = 1;
		desc.result->type = vx::FileType::Mesh;

		vx::Variant arg1;
		arg1.u64 = desc.sid.value;

		vx::Variant arg2;
		arg2.ptr = desc.pUserData;

		pushFileEvent(vx::FileEvent::Mesh_Loaded, arg1, arg2);
		result = true;

		//vx::verboseChannelPrintF(0, vx::debugPrint::Channel_FileAspect, "Loaded mesh '%s' %llu", desc.fileName, desc.sid.value);
	}
	else
	{
		//vx::verboseChannelPrintF(0, vx::debugPrint::Channel_FileAspect, "Error loading mesh '%s'", desc.fileName);
	}

	return result;
}

void FileAspect::loadFileTexture(const LoadFileOfTypeDescription &desc)
{
	desc::LoadTextureDescription loadDesc;
	loadDesc.filename = desc.fileName;
	loadDesc.fileData = desc.fileData;
	loadDesc.fileSize = desc.fileSize;
	loadDesc.sid = desc.sid;
	loadDesc.status = &desc.result->status;

	void* p = m_textureFileManager.loadTexture(loadDesc, &m_logfile);
	if (p)
	{
		desc.result->result = 1;
		desc.result->type = vx::FileType::Texture;

		vx::Variant arg1;
		arg1.u64 = desc.sid.value;

		vx::Variant arg2;
		arg2.ptr = p;

		pushFileEvent(vx::FileEvent::Texture_Loaded, arg1, arg2);
	}
}

void FileAspect::loadFileMaterial(const LoadFileOfTypeDescription &desc)
{
	LoadMaterialDescription loadDesc;
	loadDesc.shared.filename = desc.fileName;
	loadDesc.fileNameWithPath = desc.fileNameWithPath;
	loadDesc.shared.sid = desc.sid;
	loadDesc.missingFiles = desc.missingFiles;
	loadDesc.shared.status = &desc.result->status;

	auto result = loadMaterial(loadDesc);
	if (result.isValid())
	{
		desc.result->result = 1;
		desc.result->type = vx::FileType::Material;

		vx::Variant arg1;
		arg1.u64 = desc.sid.value;

		vx::Variant arg2;
		arg2.ptr = desc.pUserData;

		pushFileEvent(vx::FileEvent::Material_Loaded, arg1, arg2);

		//vx::verboseChannelPrintF(0, vx::debugPrint::Channel_FileAspect, "Loaded material '%s'", desc.fileName);
	}
	else
	{
		//vx::verboseChannelPrintF(0, vx::debugPrint::Channel_FileAspect, "Error loading material '%s'", desc.fileName);
	}
}

void FileAspect::loadFileOfType(const LoadFileOfTypeDescription &desc)
{
	switch (desc.fileType)
	{
	case vx::FileType::Mesh:
	{
		loadFileMesh(desc);
	}break;
	case vx::FileType::Texture:
	{
		loadFileTexture(desc);
	}break;
	case vx::FileType::Material:
	{
		loadFileMaterial(desc);
	}
	break;
	case vx::FileType::Scene:
	{
		if (loadFileScene(desc, false))
		{
			vx::Variant arg1;
			arg1.u64 = desc.sid.value;

			vx::Variant arg2;
			arg2.ptr = desc.pUserData; 

			pushFileEvent(vx::FileEvent::Scene_Loaded, arg1, arg2);

			desc.result->result = 1;
			desc.result->type = vx::FileType::Scene;
		}
	}break;
	case vx::FileType::EditorScene:
	{
		if (loadFileScene(desc, true))
		{
			vx::Variant arg1;
			arg1.u64 = desc.sid.value;

			vx::Variant arg2;
			arg2.ptr = desc.pUserData;

			pushFileEvent(vx::FileEvent::EditorScene_Loaded, arg1, arg2);

			desc.result->result = 1;
			desc.result->type = vx::FileType::EditorScene;
		}
	}break;
	case vx::FileType::Fbx:
	{
		if (loadFileFbx(desc))
		{
			desc.result->result = 1;
			desc.result->type = vx::FileType::Fbx;
		}
	}break;
	case vx::FileType::Animation:
	{
		if (loadFileAnimation(desc))
		{
			desc.result->result = 1;
			desc.result->type = vx::FileType::Animation;

			vx::Variant arg1;
			arg1.u64 = desc.sid.value;

			vx::Variant arg2;
			arg2.ptr = desc.pUserData;

			pushFileEvent(vx::FileEvent::Animation_Loaded, arg1, arg2);
		}
	}
	default:
		break;
	}
}

LoadFileReturnType FileAspect::loadFile(const vx::FileEntry &fileEntry, std::vector<vx::FileEntry>* missingFiles, void* pUserData)
{
	auto fileName = fileEntry.getString();
	auto fileType = fileEntry.getType();

	const char *folder = "";
	getFolderString(fileType, &folder);

	LoadFileReturnType result;

	char fileNameWithPath[64];
	auto returnCode = sprintf_s(fileNameWithPath, "%s%s", folder, fileName);
	if (returnCode == -1)
		return result;

	u32 fileSize = 0;

	SCOPE_EXIT
	{
		m_allocatorReadFile.clear();
	};

	const u8* fileData = readFile(fileNameWithPath, &fileSize);
	if (fileData == nullptr)
		return result;

	LoadFileOfTypeDescription desc;
	desc.fileName = fileName;
	desc.pUserData = pUserData;
	desc.result = &result;
	desc.sid = fileEntry.getSid();
	desc.fileData = fileData;
	desc.fileNameWithPath = fileNameWithPath;
	desc.fileSize = fileSize;
	desc.fileType = fileType;
	desc.missingFiles = missingFiles;

	loadFileOfType(desc);

	return result;
}

LoadFileReturnType FileAspect::saveFile(const FileRequest &request, vx::Variant* p)
{
	const char* fileName = request.m_fileEntry.getString();
	auto fileType = request.m_fileEntry.getType();

	vx::verboseChannelPrintF(0, vx::debugPrint::Channel_FileAspect, "Trying to save file %s", fileName);

	p->u64 = request.m_fileEntry.getSid().value;

	LoadFileReturnType result;
	result.result = 0;
	result.type = fileType;

	const char *folder = "";

	switch (fileType)
	{
	case vx::FileType::EditorScene:
		folder = s_sceneFolder;
		break;
	default:
	{
		vx::verboseChannelPrintF(0, vx::debugPrint::Channel_FileAspect, "Error, unhandled file type !");
		return result;
	}break;
	}

	char file[64];
	sprintf_s(file, "%s%s", folder, fileName);

	vx::File f;
	if (!f.create(file, vx::FileAccess::Write))
	{
		LOG_ERROR_ARGS(m_logfile, "Error opening file '%s'\n", false, file);
		vx::verboseChannelPrintF(0, vx::debugPrint::Channel_FileAspect, "Error opening file '%s'\n", file);
		return result;
	}

	SCOPE_EXIT
	{
		f.close();
	};

	u8 saveResult = 0;
	switch (fileType)
	{
	case vx::FileType::EditorScene:
	{
		auto scene = (Editor::Scene*)request.userData;
		VX_ASSERT(scene != nullptr);

		SceneFactory::saveToFile(*scene, &f);

		saveResult = 1;
		if (saveResult == 0)
		{
			vx::verboseChannelPrintF(0, vx::debugPrint::Channel_FileAspect, "Error saving scene !");
			LOG_ERROR_ARGS(m_logfile, "Error saving scene '%s'\n", false, file);
		}
		else
		{
			vx::verboseChannelPrintF(0, vx::debugPrint::Channel_FileAspect, "Saved Scene");
			LOG_ARGS(m_logfile, "Saved scene '%s'\n", false, file);
		}

		SceneFactory::deleteScene(scene);
	}
	break;
	default:
		return result;
		break;
	}

	result.result = saveResult;

	return result;
}

void FileAspect::handleSaveRequest(FileRequest* request)
{
	vx::Variant p;
	auto result = saveFile(*request, &p);
	if (result.result == 0)
	{
		if (request->m_maxRetries == 0)
		{
			LOG_WARNING_ARGS(m_logfile, "Warning: Save request timed out. '%s'\n", false, request->m_fileEntry.getString());
		}
		else
		{
			--request->m_maxRetries;

			LOG_WARNING_ARGS(m_logfile, "Warning: Retrying save request '%s'\n", false, request->m_fileEntry.getString());

			vx::lock_guard<vx::mutex> guard(m_mutexFileRequests);
			m_fileRequests.push_back(*request);
		}
	}
}

void FileAspect::retryLoadFile(const FileRequest &request, const std::vector<vx::FileEntry> &missingFiles)
{
	vx::lock_guard<vx::mutex> guard(m_mutexFileRequests);
	m_fileRequests.push_back(request);

	FileRequest loadRequest;
	for (auto &it : missingFiles)
	{
		loadRequest.m_fileEntry = it;
		m_fileRequests.push_back(loadRequest);
		LOG_ARGS(m_logfile, "Added load request '%s'\n", false, it.getString());
	}
}

void FileAspect::onLoadFileFailed(FileRequest* request, const std::vector<vx::FileEntry> &missingFiles)
{
	// failed to load
	if (request->m_maxRetries == 0)
	{
		vx::verboseChannelPrintF(0, vx::debugPrint::Channel_FileAspect, "Failed to load file %s\n", request->m_fileEntry.getString());
		LOG_WARNING_ARGS(m_logfile, "Warning: Load request timed out. '%s'\n", false, request->m_fileEntry.getString());
	}
	else
	{
		--request->m_maxRetries;

		LOG_WARNING_ARGS(m_logfile, "Warning: Retrying load request '%s'\n", false, request->m_fileEntry.getString());

		retryLoadFile(*request, missingFiles);
	}
}

void FileAspect::onExistingFile(const FileRequest* request, const vx::StringID &sid)
{
	auto fileType = request->m_fileEntry.getType();

	vx::Variant arg1;
	arg1.u64 = sid.value;

	vx::Variant arg2;
	arg2.ptr = request->userData;

	vx::FileEvent fileEvent;
	switch (fileType)
	{
	case vx::FileType::Invalid:
		break;
	case vx::FileType::Mesh:
		fileEvent = vx::FileEvent::Mesh_Existing;
		break;
	case vx::FileType::Texture:
		fileEvent = vx::FileEvent::Texture_Existing;
		break;
	case vx::FileType::Material:
		fileEvent = vx::FileEvent::Material_Existing;
		break;
	case vx::FileType::Scene:
		fileEvent = vx::FileEvent::Scene_Existing;
		break;
	case vx::FileType::EditorScene:
		fileEvent = vx::FileEvent::Scene_Existing;
		break;
	default:
		break;
	}

	pushFileEvent(fileEvent, arg1, arg2);
}

void FileAspect::handleLoadRequest(FileRequest* request, std::vector<vx::FileEntry>* missingFiles)
{
	auto fileName = request->m_fileEntry.getString();
	auto sid = vx::make_sid(fileName);
	auto it = m_loadedFiles.find(sid);
	if (it != m_loadedFiles.end())
	{
		onExistingFile(request, sid);
	}
	else
	{
		auto result = loadFile(request->m_fileEntry, missingFiles, request->userData);
		if (result.result == 0)
		{
			onLoadFileFailed(request, *missingFiles);
		}
		else
		{
			m_loadedFiles.insert(sid, fileName);
		}
	}
}

void FileAspect::handleRequest(FileRequest* request, std::vector<vx::FileEntry>* missingFiles)
{
	switch (request->m_openType)
	{
	case FileRequest::OpenType::Load:
		handleLoadRequest(request, missingFiles);
		break;
	case FileRequest::OpenType::Save:
		handleSaveRequest(request);
		break;
	default:
		break;
	}
}

void FileAspect::reset()
{

}

void FileAspect::update()
{
	vx::unique_lock<vx::mutex> lock(m_mutexFileRequests, vx::try_to_lock);
	if (!lock.owns_lock())
		return;

	auto size = m_fileRequests.size();
	lock.unlock();

	std::vector<vx::FileEntry> missingFiles;
	FileRequest request;
	while (size != 0)
	{
		lock.lock();
		request = std::move(m_fileRequests.back());
		m_fileRequests.pop_back();
		lock.unlock();

		handleRequest(&request, &missingFiles);
		missingFiles.clear();

		lock.lock();
		size = m_fileRequests.size();
		lock.unlock();
	}
}

void FileAspect::requestLoadFile(const vx::FileEntry &fileEntry, void* p)
{
	FileRequest request;
	request.m_fileEntry = fileEntry;
	request.userData = p;
	request.m_openType = FileRequest::Load;

	vx::lock_guard<vx::mutex> guard(m_mutexFileRequests);
	m_fileRequests.push_back(request);
}

void FileAspect::requestSaveFile(const vx::FileEntry &fileEntry, void* p)
{
	FileRequest request;
	request.m_fileEntry = fileEntry;
	request.userData = p;
	request.m_openType = FileRequest::Save;

	vx::verboseChannelPrintF(0, vx::debugPrint::Channel_FileAspect, "requesting save file");
	vx::lock_guard<vx::mutex> guard(m_mutexFileRequests);
	m_fileRequests.push_back(request);
}

const TextureFile* FileAspect::getTextureFile(const vx::StringID &sid) const noexcept
{
	return m_textureFileManager.getTextureFile(sid);
}

Reference<Material> FileAspect::getMaterial(const vx::StringID &sid) noexcept
{
	Reference<Material> result;

	vx::shared_lock_guard<vx::SRWMutex> guard(m_mutexLoadedFiles);
	auto it = m_sortedMaterials.find(sid);
	if (it != m_sortedMaterials.end())
		result = *it;

	return result;
}

Reference<Material> FileAspect::getMaterial(const vx::StringID &sid) const noexcept
{
	Reference<Material> result;

	vx::shared_lock_guard<vx::SRWMutex> guard(m_mutexLoadedFiles);
	auto it = m_sortedMaterials.find(sid);
	if (it != m_sortedMaterials.end())
		result = *it;

	return result;
}

const vx::MeshFile* FileAspect::getMesh(const vx::StringID &sid) const noexcept
{
	const vx::MeshFile *p = nullptr;

	vx::shared_lock_guard<vx::SRWMutex> guard(m_mutexLoadedFiles);
	auto it = m_sortedMeshes.find(sid);
	if (it != m_sortedMeshes.end())
		p = *it;

	return p;
}

const vx::AnimationFile* FileAspect::getAnimation(const vx::StringID &sid) const
{
	const vx::AnimationFile *p = nullptr;

	vx::shared_lock_guard<vx::SRWMutex> guard(m_mutexLoadedFiles);
	auto it = m_sortedAnimations.find(sid);
	if (it != m_sortedAnimations.end())
		p = *it;

	return p;
}

const char* FileAspect::getLoadedFileName(const vx::StringID &sid) const noexcept
{
	vx::shared_lock_guard<vx::SRWMutex> guard(m_mutexLoadedFiles);

	auto it = m_loadedFiles.find(sid);

	const char* result = nullptr;
	if (it != m_loadedFiles.end())
	{
		result = it->c_str();
	}

	return result;
}

bool FileAspect::releaseFile(const vx::StringID &sid, vx::FileType type)
{
	bool result = false;

	vx::lock_guard<vx::SRWMutex> lock(m_mutexLoadedFiles);
	switch (type)
	{
	case vx::FileType::Mesh:
	{
		auto it = m_sortedMeshes.find(sid);
		if (it != m_sortedMeshes.end())
		{
			m_poolMesh.destroyEntry((*it));
			result = true;
		}
	}break;
	case vx::FileType::Material:
	{
		auto it = m_sortedMaterials.find(sid);
		if (it != m_sortedMaterials.end())
		{
			m_poolMaterial.destroyEntry(it->get());
			result = true;
		}
	}break;
	case vx::FileType::Animation:
		break;
	default:
		break;
	}

	return result;
}