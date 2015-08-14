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

	struct FileRequest;

	std::vector<FileRequest> m_requests;

	ResourceManager<vx::MeshFile> m_meshData;
	ResourceManager<Material> m_materialData;
	ResourceManager<vx::Animation> m_animationData;
	ResourceManager<Graphics::Texture> m_textureData;
	vx::TaskManager* m_taskManager;
	vx::MessageManager* m_msgManager;

	void setDirectories(const std::string &dataDir);

	void sendFileMessage(const FileRequest &request);
	void pushFileMessage(vx::FileMessage type, vx::Variant arg1, vx::Variant arg2);

	void pushFileRequest(vx::FileType fileType, const vx::StringID &sid, const Event &evt, void* userData);

public:
	ResourceAspect();
	~ResourceAspect();

	bool initialize(vx::StackAllocator *mainAllocator, const std::string &dataDir, vx::TaskManager* taskManager, vx::MessageManager* msgManager);
	void shutdown();

	void update();

	void requestLoadFile(const vx::FileEntry &fileEntry, void* p);
	void requestSaveFile(const vx::FileEntry &fileEntry, void* p);

	const Graphics::Texture* getTexture(const vx::StringID &sid) const override;
	const Material* getMaterial(const vx::StringID &sid) const override;
	Material* getMaterial(const vx::StringID &sid);
	const vx::MeshFile* getMesh(const vx::StringID &sid) const override;
	const vx::Animation* getAnimation(const vx::StringID &sid) const override;

	//const char* getLoadedFileName(const vx::StringID &sid) const;
};