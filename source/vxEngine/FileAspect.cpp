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
#include "Event.h"
#include "EventTypes.h"
#include <vxLib/util/DebugPrint.h>
#include "developer.h"
#include "FileFactory.h"
#include "CreateSceneDescription.h"
#include <vxLib/File/FileHeader.h>

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

struct LoadFileSharedDescription
{
	const char* fileName;
	vx::StringID sid;
	LoadFileReturnType* result;
	void* pUserData;
};

struct FileAspect::LoadFileOfTypeDescription
{
	LoadFileSharedDescription shared;

	FileType fileType;
	const char* fileNameWithPath;
	u32 fileSize;
	const u8* fileData;
	std::vector<FileEntry>* missingFiles;
};

struct FileAspect::LoadFileMeshDescription
{
	LoadFileSharedDescription shared;

	const u8* fileData;
	u32 fileSize;
};

struct FileAspect::LoadFileMaterialDescription
{
	LoadFileSharedDescription shared;

	const char* fileNameWithPath;
	std::vector<FileEntry>* missingFiles;
};

FileAspect::FileAspect(EventManager &evtManager)
	:m_fileRequests(),
	m_eventManager(evtManager),
	m_logfile(m_clock),
	m_allocatorReadFile(),
	m_clock(),
	m_poolMesh(),
	m_poolMaterial(),
	m_poolTextureFile()
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
	m_sortedTextureFiles = vx::sorted_array<vx::StringID, TextureFile*>(maxCount, pMainAllocator);

	createPool(maxCount, &m_poolMesh, pMainAllocator);
	createPool(maxCount, &m_poolMaterial, pMainAllocator);
	createPool(maxCount, &m_poolTextureFile, pMainAllocator);

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
	destroyPool(&m_poolTextureFile);
	destroyPool(&m_poolMaterial);
	destroyPool(&m_poolMesh);

	m_sortedTextureFiles.cleanup();
	m_sortedMaterials.cleanup();
	m_sortedMeshes.cleanup();

	m_logfile.close();
	m_allocatorReadFile.release();
}

bool FileAspect::loadMesh(const vx::FileHeader &fileHeader, const char *filename, const u8 *fileData, const vx::StringID &sid, FileStatus* status)
{
	bool result = true;
	auto it = m_sortedMeshes.find(sid);
	if (it == m_sortedMeshes.end())
	{
		u16 index;
		auto meshFilePtr = m_poolMesh.createEntry(&index);
		VX_ASSERT(meshFilePtr != nullptr);

		auto marker = m_allocatorMeshData.getMarker();
		auto p = meshFilePtr->loadFromMemory(fileData, fileHeader.version, &m_allocatorMeshData);
		if (p == nullptr)
		{
			m_allocatorMeshData.clear(marker);
			return false;
		}

		it = m_sortedMeshes.insert(sid, meshFilePtr);

		*status = FileStatus::Loaded;

		LOG_ARGS(m_logfile, "Loaded Mesh '%s' %llu\n", false, filename, sid.value);
	}
	else
	{
		*status = FileStatus::Exists;
	}

	return result;
}

TextureFile* FileAspect::loadTexture(const char *filename, const u8 *ptr, u32 size, const vx::StringID &sid, FileStatus* status)
{
	TextureFile *pResult = nullptr;

	auto texIt = m_sortedTextureFiles.find(sid);
	if (texIt == m_sortedTextureFiles.end())
	{
		TextureFile textureFile;
		if (textureFile.load(ptr, size))
		{
			u16 index;
			auto textureFilePtr = m_poolTextureFile.createEntry(&index, std::move(textureFile));
			VX_ASSERT(textureFilePtr != nullptr);

			texIt = m_sortedTextureFiles.insert(sid, textureFilePtr);
			pResult = textureFilePtr;

			*status = FileStatus::Loaded;
			LOG_ARGS(m_logfile, "Loaded Texture '%s' %llu\n", false, filename, sid.value);
		}
		else
		{
			LOG_ERROR_ARGS(m_logfile, "Error loading texture '%s'\n", false, filename);
		}
	}
	else
	{
		*status = FileStatus::Exists;
		pResult = *texIt;
	}


	return pResult;
}

u8 FileAspect::loadScene(const char *filename, const u8 *ptr, const vx::StringID &sid, std::vector<FileEntry>* missingFiles, FileStatus* status, Scene* pScene)
{
	u8 result = 0;
	{
		Factory::CreateSceneDescription desc;
		desc.loadedFiles = &m_loadedFiles;
		desc.materials = &m_sortedMaterials;
		desc.meshes = &m_sortedMeshes;
		desc.pMissingFiles = missingFiles;

		if (SceneFactory::createFromMemory(desc, ptr, pScene))
		{
			*status = FileStatus::Loaded;
			LOG_ARGS(m_logfile, "Loaded scene '%s' %llu\n", false, filename, sid.value);
			result = 1;
		}
		else
		{
			LOG_ERROR_ARGS(m_logfile, "Error loading scene '%s'\n", false, filename);
		}
	}
	return result;
}

u8 FileAspect::loadScene(const char *filename, const u8 *ptr, const vx::StringID &sid, std::vector<FileEntry>* missingFiles, FileStatus* status, EditorScene* pScene)
{
	u8 result = 0;
	{
		Factory::CreateSceneDescription desc;
		desc.loadedFiles = &m_loadedFiles;
		desc.materials = &m_sortedMaterials;
		desc.meshes = &m_sortedMeshes;
		desc.pMissingFiles = missingFiles;

		if (SceneFactory::createFromMemory(desc, ptr, pScene))
		{
			*status = FileStatus::Loaded;
			LOG_ARGS(m_logfile, "Loaded scene '%s' %llu\n", false, filename, sid.value);
			result = 1;
		}
		else
		{
			LOG_ERROR_ARGS(m_logfile, "Error loading scene '%s'\n", false, filename);
		}
	}
	return result;
}

Material* FileAspect::loadMaterial(const char *filename, const char *fileNameWithPath, const vx::StringID &sid, std::vector<FileEntry>* missingFiles, FileStatus* status)
{
	Material *pResult = nullptr;

	auto it = m_sortedMaterials.find(sid);
	if (it == m_sortedMaterials.end())
	{
		Material material;

		MaterialFactoryLoadDescription desc;
		desc.fileNameWithPath = fileNameWithPath;
		desc.textureFiles = &m_sortedTextureFiles;
		desc.missingFiles = missingFiles;
		desc.material = &material;
		auto result = MaterialFactory::load(desc);

		if (result)
		{
			u16 index;
			auto materialPtr = m_poolMaterial.createEntry(&index, std::move(material));
			VX_ASSERT(materialPtr != nullptr);
			m_sortedMaterials.insert(sid, materialPtr);

			pResult = materialPtr;
			*status = FileStatus::Loaded;
			LOG_ARGS(m_logfile, "Loaded Material '%s'\n", false, filename);
		}
		else
		{
			LOG_WARNING_ARGS(m_logfile, "Error loading Material '%s'\n", false, filename);
		}
	}
	else
	{
		*status = FileStatus::Exists;
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

const u8* FileAspect::readFile(const char *file, u32 &fileSize)
{
	vx::File f;
	if (!f.open(file, vx::FileAccess::Read))
	{
		LOG_ERROR_ARGS(m_logfile, "Error opening file '%s'\n", false, file);
		return nullptr;
	}

	fileSize = f.getSize();

	VX_ASSERT(fileSize != 0);

	auto pData = (u8*)m_allocatorReadFile.allocate(fileSize);
	VX_ASSERT(pData);

	if (!f.read(pData, fileSize))
	{
		LOG_ERROR_ARGS(m_logfile, "Error reading file '%s'\n", false, file);
		return nullptr;
	}

	f.close();

	return pData;
}

void FileAspect::pushFileEvent(FileEvent code, vx::Variant arg1, vx::Variant arg2)
{
	Event e;
	e.arg1 = arg1;
	e.arg2 = arg2;
	e.type = EventType::File_Event;
	e.code = (u32)code;
	m_eventManager.addEvent(e);
}

bool FileAspect::loadFileMesh(const LoadFileMeshDescription &desc)
{
	vx::FileHeader header;
	memcpy(&header, desc.fileData, sizeof(vx::FileHeader));
	auto meshFileDataBegin = desc.fileData + sizeof(vx::FileHeader);

	if (header.magic != header.s_magic)
	{
		desc.shared.result->result = 0;
		return false;
	}

	loadMesh(header, desc.shared.fileName, meshFileDataBegin, desc.shared.sid, &desc.shared.result->status);
	desc.shared.result->result = 1;
	desc.shared.result->type = FileType::Mesh;

	vx::Variant arg1;
	arg1.u64 = desc.shared.sid.value;

	vx::Variant arg2;
	arg2.ptr = desc.shared.pUserData;

	pushFileEvent(FileEvent::Mesh_Loaded, arg1, arg2);

	return true;
}

void FileAspect::loadFileTexture(const char* fileName, u32 fileSize, const vx::StringID &sid, const u8* pData, LoadFileReturnType* result)
{
	void* p = loadTexture(fileName, pData, fileSize, sid, &result->status);
	if (p)
	{
		result->result = 1;
		result->type = FileType::Texture;

		vx::Variant arg1;
		arg1.u64 = sid.value;

		vx::Variant arg2;
		arg2.ptr = p;

		pushFileEvent(FileEvent::Texture_Loaded, arg1, arg2);
	}
}

void FileAspect::loadFileMaterial(const LoadFileMaterialDescription &desc)
{
	Material *pMaterial = loadMaterial(desc.shared.fileName, desc.fileNameWithPath, desc.shared.sid, desc.missingFiles, &desc.shared.result->status);
	if (pMaterial)
	{
		desc.shared.result->result = 1;
		desc.shared.result->type = FileType::Material;

		vx::Variant arg1;
		arg1.u64 = desc.shared.sid.value;

		vx::Variant arg2;
		arg2.ptr = desc.shared.pUserData;

		pushFileEvent(FileEvent::Material_Loaded, arg1, arg2);
	}
}

void FileAspect::loadFileOfType(const LoadFileOfTypeDescription &desc)
{
	switch (desc.fileType)
	{
	case FileType::Mesh:
	{
		LoadFileMeshDescription loadDesc;
		loadDesc.shared = desc.shared;
		loadDesc.fileSize = desc.fileSize;
		loadDesc.fileData = desc.fileData;

		loadFileMesh(loadDesc);
	}break;
	case FileType::Texture:
	{
		loadFileTexture(desc.shared.fileName, desc.fileSize, desc.shared.sid, desc.fileData, desc.shared.result);
	}break;
	case FileType::Material:
	{
		LoadFileMaterialDescription loadDesc;
		loadDesc.shared = desc.shared;
		loadDesc.fileNameWithPath = desc.fileNameWithPath;
		loadDesc.missingFiles = desc.missingFiles;

		loadFileMaterial(loadDesc);
	}
	break;
	case FileType::Scene:
	{
#if _VX_EDITOR
		if (loadScene(fileName, pData, sid, missingFiles, &result->status, (EditorScene*)pUserData) != 0)
#else
		if (loadScene(desc.shared.fileName, desc.fileData, desc.shared.sid, desc.missingFiles, &desc.shared.result->status, (Scene*)desc.shared.pUserData) != 0)
#endif
		{
			vx::Variant arg1;
			arg1.ptr = desc.shared.pUserData;

			vx::Variant arg2;
			arg2.u64 = desc.shared.sid.value;

			pushFileEvent(FileEvent::Scene_Loaded, arg1, arg2);

			desc.shared.result->result = 1;
			desc.shared.result->type = FileType::Scene;
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

	const u8* fileData = readFile(fileNameWithPath, fileSize);
	if (fileData == nullptr)
		return result;

	LoadFileOfTypeDescription desc;
	desc.shared.fileName = fileName;
	desc.shared.pUserData = pUserData;
	desc.shared.result = &result;
	desc.shared.sid = vx::make_sid(fileName);
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

	p->u64 = vx::make_sid(fileName).value;

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
		auto &scene = *(EditorScene*)request.userData;

		saveResult = FileFactory::save(&f, scene);
		//saveResult = SceneFactory::save(scene, &f);
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

void FileAspect::handleLoadRequest(FileRequest* request, std::vector<FileEntry>* missingFiles)
{
	auto result = loadFile(request->m_fileEntry, missingFiles, request->userData);
	if (result.result == 0)
	{
		onLoadFileFailed(request, *missingFiles);
	}
	else
	{
		auto fileName = request->m_fileEntry.getString();
		auto sid = vx::make_sid(fileName);
		m_loadedFiles.insert(sid, fileName);
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
	const TextureFile *p = nullptr;

	auto it = m_sortedTextureFiles.find(sid);
	if (it != m_sortedTextureFiles.end())
		p = *it;

	return p;
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