#pragma once

class Event;
class CoreAspect;
class MeshInstance;
class FileEntry;
class EventManager;
class EditorScene;
class Scene;

#include "FileEntry.h"
#include <vxLib/Allocator/StackAllocator.h>
#include "LoadFileCallback.h"
#include <vector>
#include "Logfile.h"
#include "Clock.h"
#include <vxLib/Container/sorted_array.h>
#include <mutex>
#include <vxLib/StringID.h>
#include "TextureFile.h"
#include "Material.h"
#include <vxLib\Graphics\Mesh.h>
#include "EventTypesFwd.h"
#include "Pool.h"

class VX_ALIGN(64) FileAspect
{
	static char s_textureFolder[32];
	static char s_materialFolder[32];
	static char s_sceneFolder[32];
	static char s_meshFolder[32];

	struct FileRequest
	{
		enum OpenType : U8{Load = 0, Save = 1};

		FileEntry m_fileEntry;
		void* userData{nullptr};
		U8 m_maxRetries{10};
		OpenType m_openType{};
	};

	VX_ALIGN(64) struct
	{
		std::vector<FileRequest> m_fileRequests;
		std::mutex m_mutex;
	};
	EventManager &m_eventManager;
	Logfile m_logfile;
	vx::StackAllocator m_allocReadFile;
	vx::StackAllocator m_allocatorMeshData;
	Clock m_clock;
	vx::sorted_array<vx::StringID, vx::Mesh*> m_sortedMeshes;
	vx::sorted_array<vx::StringID, Material*> m_sortedMaterials;
	vx::sorted_array<vx::StringID, TextureFile*> m_sortedTextureFiles;
	Pool<vx::Mesh> m_poolMesh;
	Pool<Material> m_poolMaterial;
	Pool<TextureFile> m_poolTextureFile;

	vx::sorted_vector<vx::StringID, std::string> m_loadedFiles;

	void getFolderString(FileType fileType, const char** folder);
	U8* readFile(const char *file, U32 &fileSize);

	void pushFileEvent(FileEvent code,vx::Variant arg1, vx::Variant arg2);

	LoadFileReturnType loadFile(const FileEntry &file, std::vector<FileEntry>* missingFiles, void* pUserData);
	bool loadMesh(const char *filename, const U8 *ptr, U8 *pMeshMemory, const vx::StringID &sid, FileStatus* status);
	TextureFile* loadTexture(const char *filename, const U8 *ptr, U32 size, const vx::StringID &sid, FileStatus* status);
	U8 loadScene(const char *filename, const U8 *ptr, const vx::StringID &sid, std::vector<FileEntry>* missingFiles, FileStatus* status, Scene* pScene);
	U8 loadScene(const char *filename, const U8 *ptr, const vx::StringID &sid, std::vector<FileEntry>* missingFiles, FileStatus* status, EditorScene* pScene);
	Material* loadMaterial(const char *filename, const char *file, const vx::StringID &sid, std::vector<FileEntry>* missingFiles, FileStatus* status);

	LoadFileReturnType saveFile(const FileRequest &request, vx::Variant* p);

	void loadFileMesh(const char* fileName, U32 fileSize, const vx::StringID &sid, U8* pData, LoadFileReturnType* result, void* pUserData);
	void loadFileTexture(const char* fileName, U32 fileSize, const vx::StringID &sid, U8* pData, LoadFileReturnType* result);
	void loadFileMaterial(const char* fileName, const char* file, const vx::StringID &sid, LoadFileReturnType* result, void* pUserData, std::vector<FileEntry>* missingFiles);
	void loadFileOfType(FileType fileType, const char *fileName, const char* file, U32 fileSize, U8* pData, LoadFileReturnType* result, void* pUserData, std::vector<FileEntry>* missingFiles);

	void handleLoadRequest(FileRequest* request, std::vector<FileEntry>* missingFiles);
	void handleSaveRequest(FileRequest* request);
	void handleRequest(FileRequest* request, std::vector<FileEntry>* missingFiles);
	void onLoadFileFailed(FileRequest* request, const std::vector<FileEntry> &missingFiles);
	void retryLoadFile(const FileRequest &request, const std::vector<FileEntry> &missingFiles);

public:
	explicit FileAspect(EventManager &evtManager);
	~FileAspect();

	bool initialize(vx::StackAllocator *pMainAllocator, const std::string &dataDir);
	void shutdown();

	void update();

	/*
	requests loading of a file, the callback can not be nullptr
	on success the callback gets called.

	if the file depends on other files, the request fails, the missing files are added to the requests and the original request is readded.
	for each successful file load the originally provided callback is called.
	a request times out after 10 unsuccessful attempts and the callback is called with type -1
	*/
	void requestLoadFile(const FileEntry &fileEntry, void* p);
	void requestSaveFile(const FileEntry &fileEntry, void* p);

	const TextureFile* getTextureFile(vx::StringID sid) const noexcept;
	Material* getMaterial(vx::StringID sid) noexcept;
	const Material* getMaterial(vx::StringID id) const noexcept;

	const vx::Mesh* getMesh(vx::StringID sid) const noexcept;

	const char* getLoadedFileName(vx::StringID sid) const noexcept;
};