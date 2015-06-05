#pragma once
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

class CoreAspect;
class MeshInstance;
class FileEntry;
class EventManager;
class Scene;

namespace Editor
{
	class Scene;
}

namespace vx
{
	struct FileHeader; 
}

#include "FileEntry.h"
#include <vxLib/Allocator/StackAllocator.h>
#include "LoadFileCallback.h"
#include <vector>
#include "Logfile.h"
#include "Timer.h"
#include <vxLib/Container/sorted_array.h>
#include <mutex>
#include <vxLib/StringID.h>
#include "TextureFile.h"
#include "Material.h"
#include <vxLib\Graphics\Mesh.h>
#include <vxEngineLib/EventTypesFwd.h>
#include "Pool.h"
#include <vxEngineLib/MeshFile.h>
#include "TextureFileManager.h"

class VX_ALIGN(64) FileAspect
{
	static char s_textureFolder[32];
	static char s_materialFolder[32];
	static char s_sceneFolder[32];
	static char s_meshFolder[32];

	struct FileRequest;

	struct LoadFileOfTypeDescription;

	struct LoadDescriptionShared;
	struct LoadMeshDescription;
	struct LoadMaterialDescription;
	struct LoadTextureDescription;

	VX_ALIGN(64) struct
	{
		std::vector<FileRequest> m_fileRequests;
		std::mutex m_mutex;
	};
	Logfile m_logfile;
	vx::StackAllocator m_allocatorReadFile;
	vx::StackAllocator m_allocatorMeshData;
	Timer m_timer;
	vx::sorted_array<vx::StringID, vx::MeshFile*> m_sortedMeshes;
	vx::sorted_array<vx::StringID, Material*> m_sortedMaterials;
	Pool<vx::MeshFile> m_poolMesh;
	Pool<Material> m_poolMaterial;
	TextureFileManager m_textureFileManager;

	vx::sorted_vector<vx::StringID, std::string> m_loadedFiles;

	void getFolderString(FileType fileType, const char** folder);
	const u8* readFile(const char *file, u32* fileSize);

	void pushFileEvent(vx::FileEvent code,vx::Variant arg1, vx::Variant arg2);

	LoadFileReturnType loadFile(const FileEntry &file, std::vector<FileEntry>* missingFiles, void* pUserData);
	bool loadMesh(const LoadMeshDescription &desc);
	Material* loadMaterial(const LoadMaterialDescription &desc);

	LoadFileReturnType saveFile(const FileRequest &request, vx::Variant* p);

	bool loadFileScene(const LoadFileOfTypeDescription &desc, bool editor);
	bool loadFileMesh(const LoadFileOfTypeDescription &desc);
	void loadFileTexture(const LoadFileOfTypeDescription &desc);
	void loadFileMaterial(const LoadFileOfTypeDescription &desc);
	void loadFileOfType(const LoadFileOfTypeDescription &desc);

	void handleLoadRequest(FileRequest* request, std::vector<FileEntry>* missingFiles);
	void handleSaveRequest(FileRequest* request);
	void handleRequest(FileRequest* request, std::vector<FileEntry>* missingFiles);
	void onLoadFileFailed(FileRequest* request, const std::vector<FileEntry> &missingFiles);
	void retryLoadFile(const FileRequest &request, const std::vector<FileEntry> &missingFiles);

public:
	FileAspect();
	~FileAspect();

	bool initialize(vx::StackAllocator *pMainAllocator, const std::string &dataDir);
	void shutdown();

	void update();

	void requestLoadFile(const FileEntry &fileEntry, void* p);
	void requestSaveFile(const FileEntry &fileEntry, void* p);

	const TextureFile* getTextureFile(vx::StringID sid) const noexcept;
	Material* getMaterial(vx::StringID sid) noexcept;
	const Material* getMaterial(vx::StringID id) const noexcept;

	const vx::MeshFile* getMesh(vx::StringID sid) const noexcept;

	const char* getLoadedFileName(vx::StringID sid) const noexcept;
};