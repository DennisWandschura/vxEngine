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

struct MaterialManager::AddTextureDesc
{
	const Graphics::Texture* texture;
	UploadManager* uploadManager;
	ID3D12Resource* textureBuffer;
	u32 slice;
};

struct MaterialManager::TryGetTextureDesc
{
	vx::StringID sid;
	const Graphics::Texture* texture;
	const ResourceAspectInterface* resourceAspect;
	UploadManager* uploadManager; 
	ID3D12Resource* textureBuffer; 
	u32* slice;
};

MaterialManager::TextureArray::TextureArray()
	:m_freelistData(),
	m_freelist(),
	m_format(),
	m_capacity(0)
{

}

MaterialManager::TextureArray::~TextureArray()
{

}

void MaterialManager::TextureArray::initialize(u32 maxTextureCount, u32 format)
{
	m_freelistData = std::make_unique<u32[]>(maxTextureCount);
	m_freelist.create((u8*)m_freelistData.get(), maxTextureCount, sizeof(u32));
	m_format = format;
	m_capacity = maxTextureCount;
}

void MaterialManager::TextureArray::addTexture(const AddTextureDesc &desc)
{
	auto &face = desc.texture->getFace(0);
	auto rowPitch = desc.texture->getFaceRowPitch(0);

	auto dim = face.getDimension();
	auto dataSize = face.getSize();
	auto data = face.getPixels();

	UploadTaskTextureDesc uploadDesc;
	uploadDesc.dst = desc.textureBuffer;
	uploadDesc.dataSize = dataSize;
	uploadDesc.data = data;
	uploadDesc.dim.x = dim.x;
	uploadDesc.dim.y = dim.y;
	uploadDesc.format = m_format;
	uploadDesc.rowPitch = rowPitch;
	uploadDesc.slice = desc.slice;
	uploadDesc.state = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	desc.uploadManager->pushUploadTexture(uploadDesc);
}

bool MaterialManager::TextureArray::tryGetTexture(const TryGetTextureDesc &desc)
{
	auto it = m_sortedEntries.find(desc.sid);
	if (it == m_sortedEntries.end())
	{
		auto ptr = (u32*)m_freelist.insertEntry((u8*)m_freelistData.get(), sizeof(u32));
		if (ptr == nullptr)
			return false;

		auto textureSlice = ptr - m_freelistData.get();

		AddTextureDesc addDesc;
		addDesc.texture = desc.texture;
		addDesc.slice = textureSlice;
		addDesc.textureBuffer = desc.textureBuffer;
		addDesc.uploadManager = desc.uploadManager;
		addTexture(addDesc);

		it = m_sortedEntries.insert(desc.sid, textureSlice);
	}

	*desc.slice = *it;
	return true;
}

MaterialManager::MaterialManager()
	:m_materialEntries(),
	m_texturesSrgba(),
	m_texturesRgba(),
	m_textureBufferSrgba(),
	m_textureBufferRgba(),
	m_textureHeap()
{

}

MaterialManager::~MaterialManager()
{

}

bool MaterialManager::createHeap(d3d::Device* device)
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

bool MaterialManager::createTextureArray(const vx::uint2 &textureResolution, u32 maxTextureCount, u32 format, d3d::Object<ID3D12Resource>* res, d3d::Device* device, u32* offset)
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
	if (!m_textureHeap.createResource(resDesc, *offset, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, nullptr, res->getAddressOf(), device))
		return false;

	*offset += thisOffset;

	return true;
}

bool MaterialManager::initialize(const vx::uint2 &textureResolution, u32 srbgaCount, u32 rgbaCount, d3d::Device* device)
{
	if (!createHeap(device))
		return false;

	u32 offset = 0;
	if (!createTextureArray(textureResolution, srbgaCount, DXGI_FORMAT_BC7_UNORM_SRGB, &m_textureBufferSrgba, device, &offset))
		return false;

	if (!createTextureArray(textureResolution, rgbaCount, DXGI_FORMAT_BC7_UNORM, &m_textureBufferRgba, device, &offset))
		return false;

	m_texturesSrgba.initialize(srbgaCount, DXGI_FORMAT_BC7_UNORM_SRGB);
	m_texturesRgba.initialize(rgbaCount, DXGI_FORMAT_BC7_UNORM);

	return true;
}

bool MaterialManager::tryGetTexture(const vx::StringID &sid, const ResourceAspectInterface* resourceAspect, UploadManager* uploadManager, u32* slice)
{
	auto texture = resourceAspect->getTexture(sid);
	auto format = texture->getFormat();

	if (format == Graphics::TextureFormat::BC7_UNORM_SRGB)
	{
		TryGetTextureDesc tryDesc;
		tryDesc.sid = sid;
		tryDesc.texture = texture;
		tryDesc.resourceAspect = resourceAspect;
		tryDesc.uploadManager = uploadManager;
		tryDesc.textureBuffer = m_textureBufferSrgba.get();
		tryDesc.slice = slice;

		if (!m_texturesSrgba.tryGetTexture(tryDesc))
			return false;
	}
	else if (format == Graphics::TextureFormat::BC7_UNORM)
	{
		TryGetTextureDesc tryDesc;
		tryDesc.sid = sid;
		tryDesc.texture = texture;
		tryDesc.resourceAspect = resourceAspect;
		tryDesc.uploadManager = uploadManager;
		tryDesc.textureBuffer = m_textureBufferRgba.get();
		tryDesc.slice = slice;

		if (!m_texturesRgba.tryGetTexture(tryDesc))
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