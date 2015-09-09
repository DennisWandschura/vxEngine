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

class UploadManager;
struct ID3D12Device;

namespace vx
{
	class StackAllocator;
}

namespace d3d
{
	class Heap;
	class ResourceManager;
	class Resource;
}

namespace Graphics
{
	class Texture;
}

#include <vxLib/math/Vector.h>
#include "Freelist.h"
#include "d3d.h"
#include <vxLib/StringID.h>
#include <vxLib/Container/sorted_array.h>

class TextureManager
{
	struct Entry;
	struct AddTextureDesc;

	vx::sorted_array<vx::StringID, u32> m_sortedEntries;
	Freelist m_freelist;
	Entry* m_entries;
	u32 m_capacity;
	u32 m_format;
	vx::StringID m_textureBuffer;

	bool createTextureBuffer(const wchar_t* id, const vx::uint3 &textureDim, u32 dxgiFormat, d3d::ResourceManager* resourceManager, ID3D12Device* device);

	void addTexture(const AddTextureDesc &desc);

	Entry* findEntry(const vx::StringID &sid) const;

public:
	TextureManager();
	TextureManager(const TextureManager&) = delete;
	TextureManager(TextureManager &&rhs);
	~TextureManager();

	TextureManager& operator=(const TextureManager&) = delete;
	TextureManager& operator=(TextureManager &&rhs);

	void getRequiredMemory(const vx::uint3 &textureDim, u32 dxgiFormat, u64* heapSizeTexture, ID3D12Device* device);

	bool initialize(vx::StackAllocator* allocator, const wchar_t* textureId, const vx::uint3 &textureDim, u32 dxgiFormat, d3d::ResourceManager* resourceManager, ID3D12Device* device);
	void shutdown();

	bool addTexture(const vx::StringID &sid, const Graphics::Texture &texture, d3d::ResourceManager* resourceManager, UploadManager* uploadManager, u32* slice);
	bool removeTexture(const vx::StringID &sid);

	//d3d::Resource* getTexture() { return m_textureBuffer; }
};