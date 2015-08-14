#pragma once

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

	bool addMaterial(const Material* material, const ResourceAspectInterface* resourceAspect, UploadManager* uploadManager);
};