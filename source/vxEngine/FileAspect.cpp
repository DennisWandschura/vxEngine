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
#include "FileAspect.h"
#include "MaterialFactory.h"
#include "SceneFactory.h"
#include <vxLib/File/File.h>
#include <vxLib/ScopeGuard.h>
#include "MeshInstance.h"
#include "Light.h"
#include "enums.h"
#include "EventManager.h"
#include <vxEngineLib/Event.h>
#include <vxEngineLib/EventTypes.h>
#include <vxLib/util/DebugPrint.h>
#include "developer.h"
#include "FileFactory.h"
#include "CreateSceneDescription.h"
#include <vxLib/File/FileHeader.h>
#include "Locator.h"

char FileAspect::s_textureFolder[32] = { "data/textures/" };
char FileAspect::s_materialFolder[32] = { "data/materials/" };
char FileAspect::s_sceneFolder[32] = { "data/scenes/" };
char FileAspect::s_meshFolder[32] = { "data/mesh/" };

namespace
{
	template<typename T>
	void createPool(u16 maxCount, Pool<T> *pool, vx::StackAllocator* allocator)
	{
		auto sizeInBytes = sizeof(T) * maxCount;

		pool->initialize(allocator->allocate(sizeInBytes, 64), maxCount);
	}

	template<typename T>
	void destroyPool(Pool<T> *pool)
	{
		pool->clear();
		pool->release();
	}
}

struct FileAspect::FileRequest
{
	enum OpenType : u8{ Load = 0, Save = 1 };

	FileEntry m_fileEntry;
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
	FileType fileType;
	const char* fileNameWithPath;
	u32 fileSize;
	const u8* fileData;
	std::vector<FileEntry>* missingFiles;
};

struct FileAspect::LoadDescriptionShared
{
	const char *filename;
	vx::StringID sid;
	FileStatus* status;
};

struct FileAspect::LoadMeshDescription
{
	LoadDescriptionShared shared;
	const vx::FileHeader* fileHeader;
	const u8 *fileData;
};

struct FileAspect::LoadMaterialDescription
{
	LoadDescriptionShared shared;
	const char *fileNameWithPath; 
	std::vector<FileEntry>* missingFiles; 
};

FileAspect::FileAspect()
	:m_fileRequests(),
	m_logfile(m_timer),
	m_allocatorReadFile(),
	m_timer(),
	m_poolMesh(),
	m_poolMaterial(),
	m_textureFileManager()
{
}

FileAspect::~FileAspect()
{
}

bool FileAspect::initialize(vx::StackAllocator *pMainAllocator, const std::string &dataDir)
{
	const auto fileMemorySize = 5 MBYTE;
	auto pFileMemory = pMainAllocator->allocate(fileMemorySize, 64);
	if (!pFileMemory)
		return false;

	m_allocatorReadFile = vx::StackAllocator(pFileMemory, fileMemorySize);

	m_logfile.create("filelog.xml");

	const u32 maxCount = 255;
	m_sortedMeshes = vx::sorted_array<vx::StringID, vx::MeshFile*>(maxCount, pMainAllocator);
	m_sortedMaterials = vx::sorted_array<vx::StringID, Material*>(maxCount, pMainAllocator);

	createPool(maxCount, &m_poolMesh, pMainAllocator);
	createPool(maxCount, &m_poolMaterial, pMainAllocator);
	m_textureFileManager.initialize(maxCount, pMainAllocator);

	const u32 textureMemorySize = 10 MBYTE;

	m_allocatorMeshData = vx::StackAllocator(pMainAllocator->allocate(textureMemorySize, 64), textureMemorySize);

	strcpy_s(s_textureFolder, (dataDir + "textures/").c_str());
	strcpy_s(s_materialFolder, (dataDir + "materials/").c_str());
	strcpy_s(s_sceneFolder, (dataDir + "scenes/").c_str());
	strcpy_s(s_meshFolder, (dataDir + "mesh/").c_str());

	return true;
}

void FileAspect::shutdown()
{
	m_textureFileManager.shutdown();


	destroyPool(&m_poolMaterial);
	destroyPool(&m_poolMesh);


	m_sortedMaterials.cleanup();
	m_sortedMeshes.cleanup();

	m_logfile.close();
	m_allocatorReadFile.release();
}

bool FileAspect::loadMesh(const LoadMeshDescription &desc)
{
	bool result = true;
	auto it = m_sortedMeshes.find(desc.shared.sid);
	if (it == m_sortedMeshes.end())
	{
		u16 index;
		auto meshFilePtr = m_poolMesh.createEntry(&index);
		VX_ASSERT(meshFilePtr != nullptr);

		auto marker = m_allocatorMeshData.getMarker();
		auto p = meshFilePtr->loadFromMemory(desc.fileData, desc.fileHeader->version, &m_allocatorMeshData);
		if (p == nullptr)
		{
			m_allocatorMeshData.clear(marker);
			return false;
		}

		it = m_sortedMeshes.insert(desc.shared.sid, meshFilePtr);

		*desc.shared.status = FileStatus::Loaded;

		LOG_ARGS(m_logfile, "Loaded Mesh '%s' %llu\n", false, desc.shared.filename, desc.shared.sid.value);
	}
	else
	{
		*desc.shared.status = FileStatus::Exists;
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
			created = SceneFactory::createFromMemory(factoryDesc, desc.fileData, (Editor::Scene*)desc.pUserData);
		}
		else
		{
			created = SceneFactory::createFromMemory(factoryDesc, desc.fileData, (Scene*)desc.pUserData);
		}

		if (created)
		{
			desc.result->status = FileStatus::Loaded;
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

Material* FileAspect::loadMaterial(const LoadMaterialDescription &desc)
{
	Material *pResult = nullptr;

	auto it = m_sortedMaterials.find(desc.shared.sid);
	if (it == m_sortedMaterials.end())
	{
		Material material;

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

			m_sortedMaterials.insert(desc.shared.sid, materialPtr);

			pResult = materialPtr;
			*desc.shared.status = FileStatus::Loaded;
			LOG_ARGS(m_logfile, "Loaded Material '%s'\n", false, desc.shared.filename);
		}
		else
		{
			LOG_WARNING_ARGS(m_logfile, "Error loading Material '%s'\n", false, desc.shared.filename);
		}
	}
	else
	{
		*desc.shared.status = FileStatus::Exists;
		pResult = (*it);
	}

	return pResult;
}

void FileAspect::getFolderString(FileType fileType, const char** folder)
{
	switch (fileType)
	{
	case FileType::Texture:
		*folder = s_textureFolder;
		break;
	case FileType::Mesh:
		*folder = s_meshFolder;
		break;
	case FileType::Material:
		*folder = s_materialFolder;
		break;
	case FileType::Scene:
		*folder = s_sceneFolder;
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

	auto evtManager = Locator::getEventManager();
	evtManager->addEvent(e);
}

bool FileAspect::loadFileMesh(const LoadFileOfTypeDescription &desc)
{
	vx::FileHeader header;
	memcpy(&header, desc.fileData, sizeof(vx::FileHeader));
	auto meshFileDataBegin = desc.fileData + sizeof(vx::FileHeader);

	if (header.magic != header.s_magic)
	{
		desc.result->result = 0;
		return false;
	}

	LoadMeshDescription loadMeshDesc;
	loadMeshDesc.fileHeader = &header;
	loadMeshDesc.shared.filename = desc.fileName;
	loadMeshDesc.fileData = meshFileDataBegin;
	loadMeshDesc.shared.sid = desc.sid;
	loadMeshDesc.shared.status = &desc.result->status;

	bool result = false;
	if (loadMesh(loadMeshDesc))
	{
		desc.result->result = 1;
		desc.result->type = FileType::Mesh;

		vx::Variant arg1;
		arg1.u64 = desc.sid.value;

		vx::Variant arg2;
		arg2.ptr = desc.pUserData;

		pushFileEvent(vx::FileEvent::Mesh_Loaded, arg1, arg2);
		result = true;
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
		desc.result->type = FileType::Texture;

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

	Material *pMaterial = loadMaterial(loadDesc);
	if (pMaterial)
	{
		desc.result->result = 1;
		desc.result->type = FileType::Material;

		vx::Variant arg1;
		arg1.u64 = desc.sid.value;

		vx::Variant arg2;
		arg2.ptr = desc.pUserData;

		pushFileEvent(vx::FileEvent::Material_Loaded, arg1, arg2);
	}
}

void FileAspect::loadFileOfType(const LoadFileOfTypeDescription &desc)
{
	switch (desc.fileType)
	{
	case FileType::Mesh:
	{
		loadFileMesh(desc);
	}break;
	case FileType::Texture:
	{
		loadFileTexture(desc);
	}break;
	case FileType::Material:
	{
		loadFileMaterial(desc);
	}
	break;
	case FileType::Scene:
	{
#if _VX_EDITOR
		if (loadFileScene(desc, true))
#else
		if (loadFileScene(desc, false))
#endif
		{
			vx::Variant arg1;
			arg1.u64 = desc.sid.value;

			vx::Variant arg2;
			arg2.ptr = desc.pUserData; 

			pushFileEvent(vx::FileEvent::Scene_Loaded, arg1, arg2);

			desc.result->result = 1;
			desc.result->type = FileType::Scene;
		}
	}break;
	default:
		break;
	}
}

LoadFileReturnType FileAspect::loadFile(const FileEntry &fileEntry, std::vector<FileEntry>* missingFiles, void* pUserData)
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

	vx::verboseChannelPrintF(0, dev::Channel_FileAspect, "Trying to save file %s\n", fileName);

	p->u64 = request.m_fileEntry.getSid().value;

	LoadFileReturnType result;
	result.result = 0;
	result.type = fileType;

	const char *folder = "";

	switch (fileType)
	{
	case FileType::Scene:
		folder = s_sceneFolder;
		break;
	default:
		return result;
		break;
	}

	char file[64];
	sprintf_s(file, "%s%s", folder, fileName);

	vx::File f;
	if (!f.create(file, vx::FileAccess::Write))
	{
		LOG_ERROR_ARGS(m_logfile, "Error opening file '%s'\n", false, file);
		return result;
	}

	SCOPE_EXIT
	{
		f.close();
	};

	u8 saveResult = 0;
	switch (fileType)
	{
	case FileType::Scene:
	{
		auto scene = (Editor::Scene*)request.userData;

		saveResult = FileFactory::save(&f, *scene);
		if (saveResult == 0)
		{
			vx::verboseChannelPrintF(0, dev::Channel_FileAspect, "Error saving scene !");
			LOG_ERROR_ARGS(m_logfile, "Error saving scene '%s'\n", false, file);
		}
		else
		{
			vx::verboseChannelPrintF(0, dev::Channel_FileAspect, "Saved Scene");
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

			std::lock_guard<std::mutex> guard(m_mutex);
			m_fileRequests.push_back(*request);
		}
	}
}

void FileAspect::retryLoadFile(const FileRequest &request, const std::vector<FileEntry> &missingFiles)
{
	std::lock_guard<std::mutex> guard(m_mutex);
	m_fileRequests.push_back(request);

	FileRequest loadRequest;
	for (auto &it : missingFiles)
	{
		loadRequest.m_fileEntry = it;
		m_fileRequests.push_back(loadRequest);
		LOG_ARGS(m_logfile, "Added load request '%s'\n", false, it.getString());
	}
}

void FileAspect::onLoadFileFailed(FileRequest* request, const std::vector<FileEntry> &missingFiles)
{
	// failed to load
	if (request->m_maxRetries == 0)
	{
		vx::verboseChannelPrintF(0, dev::Channel_FileAspect, "Failed to load file %s\n", request->m_fileEntry.getString());
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
	case FileType::Invalid:
		break;
	case FileType::Mesh:
		fileEvent = vx::FileEvent::Mesh_Existing;
		break;
	case FileType::Texture:
		fileEvent = vx::FileEvent::Texture_Existing;
		break;
	case FileType::Material:
		fileEvent = vx::FileEvent::Material_Existing;
		break;
	case FileType::Scene:
		fileEvent = vx::FileEvent::Scene_Existing;
		break;
	default:
		break;
	}

	pushFileEvent(fileEvent, arg1, arg2);
}

void FileAspect::handleLoadRequest(FileRequest* request, std::vector<FileEntry>* missingFiles)
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

void FileAspect::handleRequest(FileRequest* request, std::vector<FileEntry>* missingFiles)
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

void FileAspect::update()
{
	std::unique_lock<std::mutex> lock(m_mutex, std::try_to_lock);
	if (!lock.owns_lock())
		return;

	auto size = m_fileRequests.size();
	lock.unlock();

	std::vector<FileEntry> missingFiles;
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

void FileAspect::requestLoadFile(const FileEntry &fileEntry, void* p)
{
	FileRequest request;
	request.m_fileEntry = fileEntry;
	request.userData = p;
	request.m_openType = FileRequest::Load;

	std::lock_guard<std::mutex> guard(m_mutex);
	m_fileRequests.push_back(request);
}

void FileAspect::requestSaveFile(const FileEntry &fileEntry, void* p)
{
	FileRequest request;
	request.m_fileEntry = fileEntry;
	request.userData = p;
	request.m_openType = FileRequest::Save;

	vx::verboseChannelPrintF(0, dev::Channel_FileAspect, "requesting save file");
	std::lock_guard<std::mutex> guard(m_mutex);
	m_fileRequests.push_back(request);
}

const TextureFile* FileAspect::getTextureFile(vx::StringID sid) const noexcept
{
	return m_textureFileManager.getTextureFile(sid);
}

Material* FileAspect::getMaterial(vx::StringID sid) noexcept
{
	Material *p = nullptr;

	auto it = m_sortedMaterials.find(sid);
	if (it != m_sortedMaterials.end())
		p = *it;

	return p;
}

const Material* FileAspect::getMaterial(vx::StringID sid) const noexcept
{
	const Material *p = nullptr;

	auto it = m_sortedMaterials.find(sid);
	if (it != m_sortedMaterials.end())
		p = *it;

	return p;
}

const vx::MeshFile* FileAspect::getMesh(vx::StringID sid) const noexcept
{
	const vx::MeshFile *p = nullptr;

	auto it = m_sortedMeshes.find(sid);
	if (it != m_sortedMeshes.end())
		p = *it;

	return p;
}

const char* FileAspect::getLoadedFileName(vx::StringID sid) const noexcept
{
	auto it = m_loadedFiles.find(sid);

	const char* result = nullptr;
	if (it != m_loadedFiles.end())
	{
		result = it->c_str();
	}

	return result;
}