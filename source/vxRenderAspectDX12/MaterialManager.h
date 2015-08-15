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
	class Device;
}

#include "d3d.h"
#include "Heap.h"
#include <vxLib/math/Vector.h>
#include <vxLib/Container/sorted_vector.h>
#include <vxLib/StringID.h>
#include "Freelist.h"

class MaterialManager
{
	struct MaterialEntry;

	vx::sorted_vector<vx::StringID, MaterialEntry> m_entries;
	std::unique_ptr<u32[]> m_freelistData;
	Freelist m_freelist;
	d3d::Object<ID3D12Resource> m_textureBuffer;
	d3d::Heap m_textureHeap;
	u32 m_textureOffset;

	bool createHeap(d3d::Device* device);
	bool createSrgbTextureArray(const vx::uint2 &textureResolution, u32 maxTextureCount, d3d::Device* device);

	void addTexture(u32 slice, const vx::StringID &sid, const ResourceAspectInterface* resourceAspect, UploadManager* uploadManager);

public:
	MaterialManager();
	~MaterialManager();

	bool initialize(const vx::uint2 &textureResolution, u32 maxTextureCount, d3d::Device* device);

	bool addMaterial(const Material* material, const ResourceAspectInterface* resourceAspect, UploadManager* uploadManager, u32* slice);

	u32 getTextureCount() const { return m_entries.size(); }

	d3d::Object<ID3D12Resource>& getTextureBuffer() { return m_textureBuffer; }
};