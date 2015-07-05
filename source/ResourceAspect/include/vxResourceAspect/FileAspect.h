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

class MeshInstance;
class Scene;

namespace Editor
{
	class Scene;
}

namespace vx
{
	struct FileHeader; 
	class AnimationFile;
}

#include <vxEngineLib/FileAspectInterface.h>
#include "FileEntry.h"
#include <vxLib/Allocator/StackAllocator.h>
#include "LoadFileCallback.h"
#include <vector>
#include <vxEngineLib/Logfile.h>
#include <vxEngineLib/Timer.h>
#include <vxLib/Container/sorted_array.h>
#include <vxLib/StringID.h>
#include "TextureFileManager.h"
#include <vxLib\Graphics\Mesh.h>
#include <vxEngineLib/EventTypesFwd.h>
#include <vxEngineLib/Pool.h>
#include <vxEngineLib/SRWMutex.h>
#include <vxEngineLib/Reference.h>

class VX_ALIGN(64) FileAspect : public FileAspectInterface
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

	struct VX_ALIGN(64)
	{
		std::vector<FileRequest> m_fileRequests;
		vx::mutex m_mutexFileRequests;
		mutable vx::SRWMutex m_mutexLoadedFiles;
	};
	Logfile m_logfile;
	vx::StackAllocator m_scratchAllocator;
	vx::StackAllocator m_allocatorReadFile;
	vx::StackAllocator m_allocatorMeshData;
	Timer m_timer;
	vx::sorted_array<vx::StringID, vx::MeshFile*> m_sortedMeshes;
	vx::sorted_array<vx::StringID, Reference<Material>> m_sortedMaterials;
	vx::Pool<vx::MeshFile> m_poolMesh;
	vx::Pool<ReferenceCounted<Material>> m_poolMaterial;
	vx::Pool<vx::AnimationFile> m_poolAnimations;
	TextureFileManager m_textureFileManager;
	vx::EventManager* m_eventManager;
	physx::PxCooking* m_cooking;

	vx::sorted_vector<vx::StringID, std::string> m_loadedFiles;

	void getFolderString(vx::FileType fileType, const char** folder);
	const u8* readFile(const char *file, u32* fileSize);

	void pushFileEvent(vx::FileEvent code,vx::Variant arg1, vx::Variant arg2);

	LoadFileReturnType loadFile(const vx::FileEntry &file, std::vector<vx::FileEntry>* missingFiles, void* pUserData);
	bool loadMesh(const LoadMeshDescription &desc);
	Reference<Material> loadMaterial(const LoadMaterialDescription &desc);

	LoadFileReturnType saveFile(const FileRequest &request, vx::Variant* p);

	bool loadFileFbx(const LoadFileOfTypeDescription &desc);
	bool loadFileScene(const LoadFileOfTypeDescription &desc, bool editor);
	bool loadFileMesh(const LoadFileOfTypeDescription &desc);
	void loadFileTexture(const LoadFileOfTypeDescription &desc);
	void loadFileMaterial(const LoadFileOfTypeDescription &desc);
	void loadFileOfType(const LoadFileOfTypeDescription &desc);

	void handleLoadRequest(FileRequest* request, std::vector<vx::FileEntry>* missingFiles);
	void handleSaveRequest(FileRequest* request);
	void handleRequest(FileRequest* request, std::vector<vx::FileEntry>* missingFiles);
	void onExistingFile(const FileRequest* request, const vx::StringID &sid);
	void onLoadFileFailed(FileRequest* request, const std::vector<vx::FileEntry> &missingFiles);
	void retryLoadFile(const FileRequest &request, const std::vector<vx::FileEntry> &missingFiles);

public:
	FileAspect();
	~FileAspect();

	bool initialize(vx::StackAllocator *pMainAllocator, const std::string &dataDir, vx::EventManager* evtManager, physx::PxCooking* cooking) override;
	void shutdown() override;

	void reset() override;

	void update() override;

	void requestLoadFile(const vx::FileEntry &fileEntry, void* p) override;
	void requestSaveFile(const vx::FileEntry &fileEntry, void* p) override;

	const TextureFile* getTextureFile(const vx::StringID &sid) const noexcept override;
	Reference<Material> getMaterial(const vx::StringID &sid) noexcept override;
	Reference<Material> getMaterial(const vx::StringID &id) const noexcept override;

	const vx::MeshFile* getMesh(const vx::StringID &sid) const noexcept override;

	const char* getLoadedFileName(const vx::StringID &sid) const noexcept override;

	bool releaseFile(const vx::StringID &sid, vx::FileType type);
};