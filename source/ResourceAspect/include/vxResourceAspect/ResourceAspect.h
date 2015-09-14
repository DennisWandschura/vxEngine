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

class Event;
class Task;
class FbxFactoryInterface;

namespace physx
{
	class PxCooking;
}

namespace vx
{
	class FileEntry;
	class TaskManager;
	class MessageManager;
}

#include <vxEngineLib/ResourceAspectInterface.h>
#include <vxResourceAspect/ResourceManager.h>
#include <vxEngineLib/shared_ptr.h>
#include <vector>
#include <vxLib/Variant.h>

namespace vx
{
	enum class FileMessage : u16;
	enum class FileType : u8;
}

class ResourceAspect : public ResourceAspectInterface
{
	static char s_textureFolder[32];
	static char s_materialFolder[32];
	static char s_sceneFolder[32];
	static char s_meshFolder[32];
	static char s_animationFolder[32];
	static char s_assetFolder[32];
	static char s_audioFolder[32];

	struct FileRequest;
	struct TaskLoadFileDesc;

	std::mutex m_requestMutex;
	std::vector<FileRequest> m_requests;

	ResourceManager<vx::MeshFile> m_meshData;
	ResourceManager<Material> m_materialData;
	ResourceManager<vx::Animation> m_animationData;
	ResourceManager<Graphics::Texture> m_textureData;
	ResourceManager<AudioFile> m_audioData;
	vx::TaskManager* m_taskManager;
	vx::MessageManager* m_msgManager;
	physx::PxCooking* m_cooking;
	FbxFactoryInterface* m_fbxFactory;
	void* m_dllHandle;
	bool m_flipTextures;

	bool loadFbxFactory();

	void setDirectories(const std::string &dataDir);

	void sendFileMessage(const FileRequest &request);
	void pushFileMessage(vx::FileMessage type, vx::Variant arg1, vx::Variant arg2);

	void pushFileRequest(vx::FileType fileType, const vx::StringID &sid, const Event &evt, void* userData, std::string &&filename);

	void taskGetFileNameWithPath(const TaskLoadFileDesc &desc, const char* folder);
	void taskLoadAudio(const TaskLoadFileDesc &desc, const char* folder);
	void taskLoadScene(const TaskLoadFileDesc &desc, const char* folder, bool editor);
	void taskLoadMesh(const TaskLoadFileDesc &desc, const char* folder);
	void taskLoadMaterial(const TaskLoadFileDesc &desc, const char* folder);
	void taskLoadTexture(const TaskLoadFileDesc &desc, const char* folder);
	void taskLoadFbx(const TaskLoadFileDesc &desc, const char* folder);
	void taskSaveEditorScene(const TaskLoadFileDesc &desc, const char* folder);
	void taskSaveMeshFile(const TaskLoadFileDesc &desc, const char* folder);
	void pushTask(Task* task, vx::FileType type, const vx::StringID &sid, const Event &evt, void* p, std::string &&filename);

public:
	ResourceAspect();
	~ResourceAspect();

	bool initialize(vx::StackAllocator* mainAllocator, const std::string &dataDir, physx::PxCooking* cooking, vx::TaskManager* taskManager, vx::MessageManager* msgManager, bool flipTextures, bool editor);
	void shutdown();

	void update();

	void requestLoadFile(const vx::FileEntry &fileEntry, vx::Variant arg) override;
	void requestSaveFile(const vx::FileEntry &fileEntry, void* p);

	const Graphics::Texture* getTexture(const vx::StringID &sid) const override;
	const Material* getMaterial(const vx::StringID &sid) const override;
	Material* getMaterial(const vx::StringID &sid);
	const vx::MeshFile* getMesh(const vx::StringID &sid) const override;
	vx::MeshFile* getMesh(const vx::StringID &sid);
	const vx::Animation* getAnimation(const vx::StringID &sid) const override;
	const AudioFile* getAudioFile(const vx::StringID &sid) const override;

	ResourceManager<vx::MeshFile>* getMeshManager();
};