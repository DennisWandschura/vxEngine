#include "FileAspect.h"
#include "MaterialFactory.h"
#include "SceneFactory.h"
#include "File.h"
#include <vxLib/ScopeGuard.h>
#include "MeshInstance.h"
#include "Light.h"
#include "enums.h"
#include "EventManager.h"
#include "Event.h"
#include "EventTypes.h"

char FileAspect::s_textureFolder[32] = { "data/textures/" };
char FileAspect::s_materialFolder[32] = { "data/materials/" };
char FileAspect::s_sceneFolder[32] = { "data/scenes/" };
char FileAspect::s_meshFolder[32] = { "data/mesh/" };

FileAspect::FileAspect(EventManager &evtManager)
	:m_fileRequests(),
	m_eventManager(evtManager),
	m_logfile(m_clock),
	m_allocReadFile(),
	m_clock()
{
}

FileAspect::~FileAspect()
{
}

bool FileAspect::initialize(vx::StackAllocator *pMainAllocator, const std::string &dataDir)
{
	const auto fileMemorySize = 1 MBYTE;
	auto pFileMemory = pMainAllocator->allocate(fileMemorySize, 64);
	if (!pFileMemory)
		return false;
	m_allocReadFile = vx::StackAllocator(pFileMemory, fileMemorySize);

	m_logfile.create("filelog.xml");

	m_meshes = vx::sorted_array<vx::StringID64, vx::Mesh>(100, pMainAllocator);
	//m_scenes = vx::sorted_array<vx::StringID64, Scene>(10, pMainAllocator);
	m_materials = vx::sorted_array<vx::StringID64, Material>(100, pMainAllocator);
	m_textureFiles = vx::sorted_array<vx::StringID64, TextureFile>(100, pMainAllocator);

	const U32 textureMemorySize = 10 MBYTE;

	m_allocatorMeshData = vx::StackAllocator(pMainAllocator->allocate(textureMemorySize, 64), textureMemorySize);

	strcpy_s(s_textureFolder, (dataDir + "textures/").c_str());
	strcpy_s(s_materialFolder, (dataDir + "materials/").c_str());
	strcpy_s(s_sceneFolder, (dataDir + "scenes/").c_str());
	strcpy_s(s_meshFolder, (dataDir + "mesh/").c_str());

	return true;
}

void FileAspect::shutdown()
{
	m_textureFiles.cleanup();
	m_materials.cleanup();
	//m_scenes.cleanup();
	m_meshes.cleanup();

	m_logfile.close();
	m_allocReadFile.release();
}

bool FileAspect::loadMesh(const char *filename, const U8 *ptr, U8 *pMeshMemory, vx::StringID64 sid, FileStatus &status)
{
	bool result = true;
	auto it = m_meshes.find(sid);
	if (it == m_meshes.end())
	{
		vx::Mesh m;
		m.load(ptr, pMeshMemory);
		it = m_meshes.insert(sid, std::move(m));
		// out of memory
		VX_ASSERT(it != m_meshes.end());
		status = FileStatus::Loaded;

		LOG_ARGS(m_logfile, "Loaded Mesh '%s' %llu\n", false, filename, sid.m_value);
	}
	else
	{
		status = FileStatus::Exists;
	}

	return result;
}

TextureFile* FileAspect::loadTexture(const char *filename, const U8 *ptr, U32 size, vx::StringID64 sid, FileStatus &status)
{
	auto loadTextureFile = [](const U8 *ptr, U32 size, TextureFile* pTextureFile)
	{
		return pTextureFile->load(ptr, size);
	};

	TextureFile *pResult = nullptr;

	auto texIt = m_textureFiles.find(sid);
	if (texIt == m_textureFiles.end())
	{
		TextureFile textureFile;
		if (loadTextureFile(ptr, size, &textureFile))
		{
			texIt = m_textureFiles.insert(sid, std::move(textureFile));
			assert(texIt != m_textureFiles.end());
			pResult = &*texIt;

			status = FileStatus::Loaded;
			LOG_ARGS(m_logfile, "Loaded Texture '%s' %llu\n", false, filename, sid.m_value);
		}
		else
		{
			LOG_ERROR_ARGS(m_logfile, "Error loading texture '%s'\n", false, filename);
		}
	}
	else
	{
		status = FileStatus::Exists;
		pResult = &*texIt;
	}


	return pResult;
}

U8 FileAspect::loadScene(const char *filename, const U8 *ptr, vx::StringID64 sid, std::vector<FileEntry> &missingFiles, FileStatus &status, Scene* pScene)
{
	U8 result = 0;
	{
		if (SceneFactory::load(ptr, m_meshes, m_materials, &missingFiles, pScene))
		{
			status = FileStatus::Loaded;
			LOG_ARGS(m_logfile, "Loaded scene '%s' %llu\n", false, filename, sid.m_value);
			result = 1;
		}
		else
		{
			LOG_ERROR_ARGS(m_logfile, "Error loading scene '%s'\n", false, filename);
		}
	}
	return result;
}

U8 FileAspect::loadScene(const char *filename, const U8 *ptr, vx::StringID64 sid, std::vector<FileEntry> &missingFiles, FileStatus &status, EditorScene* pScene)
{
	U8 result = 0;
	{
		if (SceneFactory::load(ptr, m_meshes, m_materials, &missingFiles, pScene))
		{
			status = FileStatus::Loaded;
			LOG_ARGS(m_logfile, "Loaded scene '%s' %llu\n", false, filename, sid.m_value);
			result = 1;
		}
		else
		{
			LOG_ERROR_ARGS(m_logfile, "Error loading scene '%s'\n", false, filename);
		}
	}
	return result;
}

Material* FileAspect::loadMaterial(const char *filename, const char *file, vx::StringID64 sid, std::vector<FileEntry> &missingFiles, FileStatus &status)
{
	Material *pResult = nullptr;

	auto it = m_materials.find(sid);
	if (it == m_materials.end())
	{
		auto result = MaterialFactory::load(file, m_textureFiles, missingFiles);

		if (result.first)
		{
			it = m_materials.insert(sid, std::move(result.second));
			pResult = &(*it);
			status = FileStatus::Loaded;
			LOG_ARGS(m_logfile, "Loaded Material '%s'\n", false, filename);
		}
		else
		{
			LOG_WARNING_ARGS(m_logfile, "Error loading Material '%s'\n", false, filename);
		}
	}
	else
	{
		status = FileStatus::Exists;
		pResult = &(*it);
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

U8* FileAspect::readFile(const char *file, U32 &fileSize)
{
	File f;
	if (!f.open(file, FileAccess::Read))
	{
		LOG_ERROR_ARGS(m_logfile, "Error opening file '%s'\n", false, file);
		return nullptr;
	}

	fileSize = f.getSize();

	VX_ASSERT(fileSize != 0);

	auto pData = (U8*)m_allocReadFile.allocate(fileSize);
	VX_ASSERT(pData);

	// read file contents into memory
	if (!f.read(pData, fileSize))
	{
		LOG_ERROR_ARGS(m_logfile, "Error reading file '%s'\n", false, file);
		return nullptr;
	}

	// close file after reading its contents into memory
	f.close();

	return pData;
}

void FileAspect::pushFileEvent(FileEvent code, Variant arg1, Variant arg2)
{
	Event e;
	e.arg1 = arg1;
	e.arg2 = arg2;
	e.type = EventType::File_Event;
	e.code = (U32)code;
	m_eventManager.addEvent(e);
}

LoadFileReturnType FileAspect::loadFile(const FileEntry &fileEntry, std::vector<FileEntry> &missingFiles, void* pUserData)
{
	auto fileName = fileEntry.getString();
	auto fileType = fileEntry.getType();

	const char *folder = "";
	getFolderString(fileType, &folder);

	char file[64];
	sprintf_s(file, "%s%s", folder, fileName);

	LoadFileReturnType result;
	U32 fileSize = 0;

	SCOPE_EXIT
	{
		m_allocReadFile.clear();
	};

	U8* pData = readFile(file, fileSize);
	if (pData == nullptr)
		return result;

	auto sid = vx::make_sid(fileName);

	switch (fileType)
	{
	case FileType::Mesh:
	{
		auto pMeshMemory = m_allocatorMeshData.allocate(fileSize, 8);
		VX_ASSERT(pMeshMemory);

		loadMesh(fileName, pData, pMeshMemory, sid, result.status);
		result.result = 1;
		result.type = FileType::Mesh;

		pushFileEvent(FileEvent::Mesh_Loaded, sid, pUserData);

		//p->sid = sid;
	}break;
	case FileType::Texture:
	{
		void* p = nullptr;
		if ((p = loadTexture(fileName, pData, fileSize, sid, result.status)))
		{
			result.result = 1;
			result.type = FileType::Texture;

			pushFileEvent(FileEvent::Texture_Loaded, sid, p);
		}
	}break;
	case FileType::Material:
	{
		Material *pMaterial = loadMaterial(fileName, file, sid, missingFiles, result.status);
		if (pMaterial)
		{
			result.result = 1;
			//p->sid = sid;
			result.type = FileType::Material;

			pushFileEvent(FileEvent::Material_Loaded, sid, pUserData);
		}
	}
	break;
	case FileType::Scene:
	{
#if _VX_EDITOR
		if (loadScene(fileName, pData, sid, missingFiles, result.status, (EditorScene*)pUserData) != 0)
#else
		if (loadScene(fileName, pData, sid, missingFiles, result.status, (Scene*)pUserData) != 0)
#endif
		{
			pushFileEvent(FileEvent::Scene_Loaded, pUserData, sid);

			result.result = 1;
			result.type = FileType::Scene;
		}
	}break;
	default:
		break;
	}

	return result;
}

LoadFileReturnType FileAspect::saveFile(FileRequest &request, Variant* p)
{
	const char* fileName = request.m_fileEntry.getString();
	auto fileType = request.m_fileEntry.getType();
	auto pUserData = request.userData;

	p->sid = vx::make_sid(fileName);

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

	File f;
	if (!f.open(file, FileAccess::Write))
	{
		LOG_ERROR_ARGS(m_logfile, "Error opening file '%s'\n", false, file);
		return result;
	}

	SCOPE_EXIT
	{
		f.close();
	};

	U8 saveResult = 0;
	switch (fileType)
	{
	case FileType::Scene:
	{
		assert(false);
		//saveResult = SceneFactory::save((const Scene*)pUserData, &f);
		if (saveResult == 0)
		{
			LOG_ERROR_ARGS(m_logfile, "Error saving scene '%s'\n", false, file);
		}
		else
		{
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

void FileAspect::handleSaveRequest(FileRequest &request)
{
	Variant p;
	auto result = saveFile(request, &p);
	if (result.result != 0)
	{
		// success
		//request.m_callback(p, result, request.userData);
	}
	else
	{
		if (request.m_maxRetries == 0)
		{
			result.type = FileType::Invalid;
			//request.m_callback(p, result, request.userData);

			LOG_WARNING_ARGS(m_logfile, "Warning: Save request timed out. '%s'\n", false, request.m_fileEntry.getString());
		}
		else
		{
			--request.m_maxRetries;

			LOG_WARNING_ARGS(m_logfile, "Warning: Retrying save request '%s'\n", false, request.m_fileEntry.getString());

			std::lock_guard<std::mutex> guard(m_mutex);
			m_fileRequests.push_back(request);
		}
	}
}

void FileAspect::handleLoadRequest(FileRequest &request, std::vector<FileEntry> &missingFiles)
{
	auto result = loadFile(request.m_fileEntry, missingFiles, request.userData);
	if (result.result != 0)
	{
		// if callback is present, call it
		//request.m_callback(p, result, request.userData);
	}
	else
	{
		// failed to load
		if (request.m_maxRetries == 0)
		{
			result.type = FileType::Invalid;
			//request.m_callback(p, result, request.userData);

			LOG_WARNING_ARGS(m_logfile, "Warning: Load request timed out. '%s'\n", false, request.m_fileEntry.getString());
		}
		else
		{
			--request.m_maxRetries;

			LOG_WARNING_ARGS(m_logfile, "Warning: Retrying load request '%s'\n", false, request.m_fileEntry.getString());

			std::lock_guard<std::mutex> guard(m_mutex);
			m_fileRequests.push_back(request);

			FileRequest loadRequest;
			//loadRequest.m_callback = request.m_callback;
			for (auto &it : missingFiles)
			{
				loadRequest.m_fileEntry = it;
				m_fileRequests.push_back(loadRequest);
				LOG_ARGS(m_logfile, "Added load request '%s'\n", false, it.getString());
			}
		}
	}
}

void FileAspect::handleRequest(FileRequest &request, std::vector<FileEntry> &missingFiles)
{
	switch (request.m_openType)
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

		handleRequest(request, missingFiles);
		missingFiles.clear();

		lock.lock();
		size = m_fileRequests.size();
		lock.unlock();
	}
}

void FileAspect::requestLoadFile(const FileEntry &fileEntry, void* p)
{
//	assert(callback);

	FileRequest request;
	request.m_fileEntry = fileEntry;
	//request.m_callback = callback;
	request.userData = p;
	request.m_openType = FileRequest::Load;

	std::lock_guard<std::mutex> guard(m_mutex);
	m_fileRequests.push_back(request);
}

void FileAspect::requestSaveFile(const FileEntry &fileEntry, void* p)
{
	FileRequest request;
	request.m_fileEntry = fileEntry;
	//request.m_callback = callback;
	request.userData = p;
	request.m_openType = FileRequest::Save;

	std::lock_guard<std::mutex> guard(m_mutex);
	m_fileRequests.push_back(request);
}

const TextureFile* FileAspect::getTextureFile(const vx::StringID64 &sid) const noexcept
{
	const TextureFile *p = nullptr;

	auto it = m_textureFiles.find(sid);
	if (it != m_textureFiles.end())
		p = &*it;

	return p;
}

Material* FileAspect::getMaterial(const vx::StringID64 &sid) noexcept
{
	Material *p = nullptr;

	auto it = m_materials.find(sid);
	if (it != m_materials.end())
		p = &*it;

	return p;
}

const Material* FileAspect::getMaterial(const vx::StringID64 &sid) const noexcept
{
	const Material *p = nullptr;

	auto it = m_materials.find(sid);
	if (it != m_materials.end())
		p = &*it;

	return p;
}

const vx::Mesh* FileAspect::getMesh(const vx::StringID64 &sid) const noexcept
{
	const vx::Mesh *p = nullptr;

	auto it = m_meshes.find(sid);
	if (it != m_meshes.end())
		p = &*it;

	return p;
}

/*Scene* FileAspect::getScene(const vx::StringID64 &sid) noexcept
{
	Scene *p = nullptr;

	auto it = m_scenes.find(sid);
	if (it != m_scenes.end())
		p = &*it;

	return p;
}

const Scene* FileAspect::getScene(const vx::StringID64 &sid) const noexcept
{
	const Scene *p = nullptr;

	auto it = m_scenes.find(sid);
	if (it != m_scenes.end())
		p = &*it;

	return p;
}*/