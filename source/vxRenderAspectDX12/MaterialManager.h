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

struct ID3D12Resource;

class Material;
class ResourceAspectInterface;
class UploadManager;

namespace d3d
{
	class ResourceManager;
}

#include "d3d.h"
#include "Heap.h"
#include <vxLib/math/Vector.h>
#include <vxLib/Container/sorted_vector.h>
#include <vxLib/StringID.h>
#include "TextureManager.h"

class MaterialManager
{
	struct MaterialEntry;

	vx::sorted_vector<vx::StringID, MaterialEntry> m_materialEntries;
	TextureManager m_texturesSrgba;
	TextureManager m_texturesRgba;
	d3d::ResourceManager* m_resourceManager;

	bool tryGetTexture(const vx::StringID &sid, const ResourceAspectInterface* resourceAspect, UploadManager* uploadManager, u32* slice);

public:
	MaterialManager();
	~MaterialManager();

	void getRequiredMemory(const vx::uint3 &dimSrgb, const vx::uint3 &dimRgb, u64* heapSizeTexture, u32* textureCount, ID3D12Device* device);

	bool initialize(const vx::uint3 &dimSrgb, const vx::uint3 &dimRgb, vx::StackAllocator* allocator, d3d::ResourceManager* resourceManager, ID3D12Device* device);
	void shutdown();

	bool addMaterial(const Material* material, const ResourceAspectInterface* resourceAspect, UploadManager* uploadManager, u32* index, u32* slices);
	bool addMaterial(const vx::StringID &materialSid, const ResourceAspectInterface* resourceAspect, UploadManager* uploadManager, u32* index, u32* slices);
};