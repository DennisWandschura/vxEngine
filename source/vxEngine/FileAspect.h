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

	struct FileRequest;

	VX_ALIGN(64) struct
	{
		std::vector<FileRequest> m_fileRequests;
		std::mutex m_mutex;
	};
	EventManager &m_eventManager;
	Logfile m_logfile;
	vx::StackAllocator m_allocatorReadFile;
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
	u8* readFile(const char *file, u32 &fileSize);

	void pushFileEvent(FileEvent code,vx::Variant arg1, vx::Variant arg2);

	LoadFileReturnType loadFile(const FileEntry &file, std::vector<FileEntry>* missingFiles, void* pUserData);
	bool loadMesh(const char *filename, const u8 *ptr, u8 *pMeshMemory, const vx::StringID &sid, FileStatus* status);
	TextureFile* loadTexture(const char *filename, const u8 *ptr, u32 size, const vx::StringID &sid, FileStatus* status);
	u8 loadScene(const char *filename, const u8 *ptr, const vx::StringID &sid, std::vector<FileEntry>* missingFiles, FileStatus* status, Scene* pScene);
	u8 loadScene(const char *filename, const u8 *ptr, const vx::StringID &sid, std::vector<FileEntry>* missingFiles, FileStatus* status, EditorScene* pScene);
	Material* loadMaterial(const char *filename, const char *file, const vx::StringID &sid, std::vector<FileEntry>* missingFiles, FileStatus* status);

	LoadFileReturnType saveFile(const FileRequest &request, vx::Variant* p);

	void loadFileMesh(const char* fileName, u32 fileSize, const vx::StringID &sid, u8* pData, LoadFileReturnType* result, void* pUserData);
	void loadFileTexture(const char* fileName, u32 fileSize, const vx::StringID &sid, u8* pData, LoadFileReturnType* result);
	void loadFileMaterial(const char* fileName, const char* file, const vx::StringID &sid, LoadFileReturnType* result, void* pUserData, std::vector<FileEntry>* missingFiles);
	void loadFileOfType(FileType fileType, const char *fileName, const char* file, u32 fileSize, u8* pData, LoadFileReturnType* result, void* pUserData, std::vector<FileEntry>* missingFiles);

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

	void requestLoadFile(const FileEntry &fileEntry, void* p);
	void requestSaveFile(const FileEntry &fileEntry, void* p);

	const TextureFile* getTextureFile(vx::StringID sid) const noexcept;
	Material* getMaterial(vx::StringID sid) noexcept;
	const Material* getMaterial(vx::StringID id) const noexcept;

	const vx::Mesh* getMesh(vx::StringID sid) const noexcept;

	const char* getLoadedFileName(vx::StringID sid) const noexcept;
};