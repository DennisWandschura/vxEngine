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

#include "TextureManager.h"
#include <vxLib/Allocator/StackAllocator.h>
#include <d3d12.h>
#include "Heap.h"
#include <vxEngineLib/Graphics/Texture.h>
#include "UploadManager.h"
#include "ResourceManager.h"

struct TextureManager::Entry
{
	vx::StringID sid;
	u16 slice;
	u16 refCount;
	u32 flag;
};

struct TextureManager::AddTextureDesc
{
	const Graphics::Texture* texture;
	UploadManager* uploadManager;
	ID3D12Resource* textureBuffer;
	u32 slice;
};

TextureManager::TextureManager()
	:m_freelist(),
	m_entries(nullptr),
	m_capacity(0),
	m_format(0),
	m_textureBuffer()
{

}

TextureManager::~TextureManager()
{
	m_entries = nullptr;
}

void TextureManager::getRequiredMemory(const vx::uint3 &textureDim, u32 dxgiFormat, u64* heapSizeTexture, ID3D12Device* device)
{
	D3D12_RESOURCE_DESC resDesc;
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Alignment = 64 KBYTE;
	resDesc.Width = textureDim.x;
	resDesc.Height = textureDim.y;
	resDesc.DepthOrArraySize = textureDim.z;
	resDesc.MipLevels = 1;
	resDesc.Format = (DXGI_FORMAT)dxgiFormat;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	auto allocInfo = device->GetResourceAllocationInfo(1, 1, &resDesc);

	*heapSizeTexture += allocInfo.SizeInBytes;
}

bool TextureManager::createTextureBuffer(const wchar_t* id, const vx::uint3 &textureDim, u32 dxgiFormat, d3d::ResourceManager* resourceManager, ID3D12Device* device)
{
	D3D12_RESOURCE_DESC resDesc;
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Alignment = 64 KBYTE;
	resDesc.Width = textureDim.x;
	resDesc.Height = textureDim.y;
	resDesc.DepthOrArraySize = textureDim.z;
	resDesc.MipLevels = 1;
	resDesc.Format = (DXGI_FORMAT)dxgiFormat;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	auto allocInfo = device->GetResourceAllocationInfo(1, 1, &resDesc);

	CreateResourceDesc desc;
	desc.clearValue = nullptr;
	desc.resDesc = &resDesc;
	desc.size = allocInfo.SizeInBytes;
	desc.state = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

	m_textureBuffer = resourceManager->createTexture(id, desc);
	if (m_textureBuffer == nullptr)
		return false;

	auto textureFormat = Graphics::dxgiFormatToTextureFormat(dxgiFormat);
	auto textureSize = Graphics::getTextureSize(textureFormat, vx::uint2(textureDim.x, textureDim.y)) * textureDim.z;
	VX_ASSERT(textureSize != 0);
	auto alignedTextureSize = d3d::getAlignedSize(textureSize, 64u KBYTE);

	return true;
}

bool TextureManager::initialize(vx::StackAllocator* allocator, const wchar_t* textureId, const vx::uint3 &textureDim, u32 dxgiFormat, d3d::ResourceManager* resourceManager, ID3D12Device* device)
{
	if (!createTextureBuffer(textureId, textureDim, dxgiFormat, resourceManager, device))
		return false;

	auto capacity = textureDim.z;
	auto entries = (Entry*)allocator->allocate(sizeof(Entry) * capacity, 4);
	if (entries == nullptr)
		return false;

	m_entries = entries;
	m_freelist.create((u8*)m_entries, capacity, sizeof(Entry));
	m_capacity = capacity;
	m_format = dxgiFormat;

	for (u32 i = 0; i < m_capacity; ++i)
	{
		m_entries[i].flag = 0;
	}

	return true;
}

void TextureManager::shutdown()
{
	m_entries = nullptr;
	m_capacity = 0;
	m_freelist.destroy();
	m_textureBuffer = nullptr;
}

void TextureManager::addTexture(const AddTextureDesc &desc)
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

TextureManager::Entry* TextureManager::findEntry(const vx::StringID &sid) const
{
	auto cap = m_capacity;
	Entry* entry = nullptr;

	for (u32 i = 0; i < cap; ++i)
	{
		auto cmp = (m_entries[i].flag != 0) && (m_entries[i].sid == sid);
		if (cmp)
		{
			entry = &m_entries[i];
			break;
		}
	}

	return entry;
}

bool TextureManager::addTexture(const vx::StringID &sid, const Graphics::Texture &texture, UploadManager* uploadManager, u32* slice)
{
	Entry* entry = findEntry(sid);

	if (entry)
	{
		++entry->refCount;
		*slice = entry->slice;
		return true;
	}

	auto ptr = (Entry*)m_freelist.insertEntry((u8*)m_entries, sizeof(Entry));
	if (ptr == nullptr)
		return false;

	auto llIndex = ptr - m_entries;
	u32 index = static_cast<u32>(llIndex);

	m_entries[index].sid = sid;
	m_entries[index].slice = index;
	VX_ASSERT(m_entries[index].slice == index);

	m_entries[index].refCount = 1;
	m_entries[index].flag = 1;

	auto &face = texture.getFace(0);
	auto dim = face.getDimension();
	
	UploadTaskTextureDesc desc;
	desc.data = face.getPixels();
	desc.dataSize = face.getSize();
	desc.dim.x = dim.x;
	desc.dim.y = dim.y;
	desc.dst = m_textureBuffer;
	desc.format = m_format;
	desc.rowPitch = texture.getFaceRowPitch(0);
	desc.slice = index;
	desc.state = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	uploadManager->pushUploadTexture(desc);

	*slice = index;
	return true;
}

bool TextureManager::removeTexture(const vx::StringID &sid)
{
	Entry* entry = findEntry(sid);
	if (entry)
	{
		--entry->refCount;

		if (entry->refCount == 0)
		{
			entry->sid = 0;
			entry->flag = 0;
			
			m_freelist.eraseEntry((u8*)entry, (u8*)m_entries, sizeof(Entry), m_capacity);
		}
	}

	return (entry != nullptr);
}