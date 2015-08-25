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

#include "MaterialManager.h"
#include <d3d12.h>
#include <vxEngineLib/Material.h>
#include <vxEngineLib/ResourceAspectInterface.h>
#include <vxEngineLib/Graphics/Texture.h>
#include "UploadManager.h"

struct MaterialManager::MaterialEntry
{
	u32 diffuseSlice;
	u32 normalSlice;
	u32 surfaceSlice;
};

MaterialManager::MaterialManager()
	:m_materialEntries(),
	m_texturesSrgba(),
	m_texturesRgba(),
	m_textureHeap()
{

}

MaterialManager::~MaterialManager()
{

}

bool MaterialManager::createHeap(ID3D12Device* device)
{
	D3D12_HEAP_PROPERTIES props
	{
		D3D12_HEAP_TYPE_DEFAULT,
		D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		D3D12_MEMORY_POOL_UNKNOWN,
		0,
		0
	};

	D3D12_HEAP_DESC textureHeapDesc;
	textureHeapDesc.Alignment = 64u KBYTE;
	textureHeapDesc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES;
	textureHeapDesc.Properties = props;
	textureHeapDesc.SizeInBytes = d3d::getAlignedSize(512u MBYTE, 64u KBYTE);
	if (!m_textureHeap.create(textureHeapDesc, device))
		return false;

	return true;
}

bool MaterialManager::createTextureArray(const vx::uint2 &textureResolution, u32 maxTextureCount, u32 format, d3d::Object<ID3D12Resource>* res, ID3D12Device* device, u32* offset)
{
	auto thisOffset = d3d::getAlignedSize(4 * textureResolution.x * textureResolution.y, 64u KBYTE);

	D3D12_RESOURCE_DESC resDesc;
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Alignment = 64 KBYTE;
	resDesc.Width = textureResolution.x;
	resDesc.Height = textureResolution.y;
	resDesc.DepthOrArraySize = maxTextureCount;
	resDesc.MipLevels = 1;
	resDesc.Format = (DXGI_FORMAT)format;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	d3d::HeapCreateResourceDesc desc;
	desc.clearValue = nullptr;
	desc.desc = &resDesc;
	desc.resource = res->getAddressOf();
	desc.state = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	if (!m_textureHeap.createResource(desc))
		return false;

	*offset += thisOffset;

	return true;
}

bool MaterialManager::initialize(const vx::uint2 &textureResolution, u32 srgbaCount, u32 rgbaCount, vx::StackAllocator* allocator, d3d::ResourceManager* resourceManager, ID3D12Device* device)
{
	if (!createHeap(device))
		return false;

	if (!m_texturesSrgba.initialize(allocator, "srgbTexture", vx::uint3(textureResolution, srgbaCount), DXGI_FORMAT_BC7_UNORM_SRGB, &m_textureHeap, resourceManager, device))
		return false;

	if (!m_texturesRgba.initialize(allocator, "rgbTexture", vx::uint3(textureResolution, rgbaCount), DXGI_FORMAT_BC7_UNORM, &m_textureHeap, resourceManager, device))
		return false;

	return true;
}

void MaterialManager::shutdown()
{
	m_materialEntries.clear();
	m_texturesRgba.shutdown();
	m_texturesSrgba.shutdown();

	m_textureHeap.destroy();
}

bool MaterialManager::tryGetTexture(const vx::StringID &sid, const ResourceAspectInterface* resourceAspect, UploadManager* uploadManager, u32* slice)
{
	auto texture = resourceAspect->getTexture(sid);
	auto format = texture->getFormat();

	if (format == Graphics::TextureFormat::BC7_UNORM_SRGB)
	{
		if(!m_texturesSrgba.addTexture(sid, *texture, uploadManager, slice))
			return false;
	}
	else if (format == Graphics::TextureFormat::BC7_UNORM)
	{
		if (!m_texturesRgba.addTexture(sid, *texture, uploadManager, slice))
			return false;
	}
	else
	{
		return false;
	}

	return true;
}

bool MaterialManager::addMaterial(const Material* material, const ResourceAspectInterface* resourceAspect, UploadManager* uploadManager, u32* index, u32* slices)
{
	auto materialSid = material->getSid();
	auto iterMaterial = m_materialEntries.find(materialSid);
	if (iterMaterial == m_materialEntries.end())
	{
		auto diffuseTextureSid = material->m_textureSid[0];
		auto normalTextureSid = material->m_textureSid[1];
		auto surfaceTextureSid = material->m_textureSid[2];

		u32 diffuseSlice = 0;
		if (!tryGetTexture(diffuseTextureSid, resourceAspect, uploadManager, &diffuseSlice))
			return false;

		u32 normalSlice = 0;
		if (!tryGetTexture(normalTextureSid, resourceAspect, uploadManager, &normalSlice))
			return false;

		u32 surfaceSlice = 0;
		if (!tryGetTexture(surfaceTextureSid, resourceAspect, uploadManager, &surfaceSlice))
			return false;

		MaterialEntry entry;
		entry.diffuseSlice = diffuseSlice;
		entry.normalSlice = normalSlice;
		entry.surfaceSlice = surfaceSlice;
		iterMaterial = m_materialEntries.insert(materialSid, entry);
	}

	*index = iterMaterial->diffuseSlice;
	*slices = iterMaterial->diffuseSlice | (iterMaterial->normalSlice << 8) | (iterMaterial->surfaceSlice << 16);

	return true;
}

bool MaterialManager::addMaterial(const vx::StringID &materialSid, const ResourceAspectInterface* resourceAspect, UploadManager* uploadManager, u32* index, u32* slices)
{
	auto iterMaterial = m_materialEntries.find(materialSid);
	if (iterMaterial == m_materialEntries.end())
	{
		auto material = resourceAspect->getMaterial(materialSid);

		auto diffuseTextureSid = material->m_textureSid[0];
		auto normalTextureSid = material->m_textureSid[1];
		auto surfaceTextureSid = material->m_textureSid[2];

		u32 diffuseSlice = 0;
		if (!tryGetTexture(diffuseTextureSid, resourceAspect, uploadManager, &diffuseSlice))
			return false;

		u32 normalSlice = 0;
		if (!tryGetTexture(normalTextureSid, resourceAspect, uploadManager, &normalSlice))
			return false;

		u32 surfaceSlice = 0;
		if (!tryGetTexture(surfaceTextureSid, resourceAspect, uploadManager, &surfaceSlice))
			return false;

		MaterialEntry entry;
		entry.diffuseSlice = diffuseSlice;
		entry.normalSlice = normalSlice;
		entry.surfaceSlice = surfaceSlice;
		iterMaterial = m_materialEntries.insert(materialSid, entry);
	}

	*index = iterMaterial->diffuseSlice;
	*slices = iterMaterial->diffuseSlice | (iterMaterial->normalSlice << 8) | (iterMaterial->surfaceSlice << 16);

	return true;
}