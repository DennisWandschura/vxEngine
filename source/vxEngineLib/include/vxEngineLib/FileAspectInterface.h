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

class Material;

namespace Graphics
{
	class Texture;
}

template<typename T>
class Reference;

#include <vxLib/types.h>

namespace vx
{
	class StackAllocator;
	class MessageManager;
	class FileEntry;
	struct StringID;
	class MeshFile;
	struct Animation;

	enum class FileType : u8;
}

namespace physx
{
	class PxCooking;
}

#include <string>

class FileAspectInterface
{
public:
	virtual ~FileAspectInterface(){}

	virtual bool initialize(vx::StackAllocator *pMainAllocator, const std::string &dataDir, vx::MessageManager* msgManager, physx::PxCooking* cooking) = 0;
	virtual void shutdown() = 0;

	virtual void reset() = 0;

	virtual void update() = 0;

	virtual void requestLoadFile(const vx::FileEntry &fileEntry, void* p) = 0;
	virtual void requestSaveFile(const vx::FileEntry &fileEntry, void* p) = 0;

	virtual const Graphics::Texture* getTexture(const vx::StringID &sid) const noexcept = 0;
	virtual Material* getMaterial(const vx::StringID &sid) noexcept = 0;
	virtual const Material* getMaterial(const vx::StringID &id) const noexcept = 0;

	virtual const vx::MeshFile* getMesh(const vx::StringID &sid) const noexcept = 0;
	virtual const vx::Animation* getAnimation(const vx::StringID &sid) const = 0;

	virtual const char* getLoadedFileName(const vx::StringID &sid) const noexcept = 0;

	virtual bool releaseFile(const vx::StringID &sid, vx::FileType type) = 0;
};