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
#include <vxEngineLib/Material.h>
#include <vxGL/Buffer.h>
#include "gl/ObjectManager.h"
#include <vxEngineLib/FileAspectInterface.h>
#include <vxEngineLib/Graphics/Texture.h>

namespace MaterialManagerCpp
{
	void uploadTextureData(const u8* pixels, u32 w, u32 h, u32 index, u8 isCommitted, vx::gl::Texture* texture)
	{
		if (isCommitted == 0)
		{
			vx::gl::TextureCommitDescription desc;
			desc.miplevel = 0;
			desc.offset = vx::uint3(0, 0, index);
			desc.size = vx::uint3(w, h, 1);
			desc.commit = 1;
			texture->commit(desc);
		}

		vx::gl::TextureSubImageDescription desc;
		desc.size = vx::uint3(w, h, 1);
		desc.miplevel = 0;
		desc.offset = vx::uint3(0, 0, index);
		desc.dataType = vx::gl::DataType::Unsigned_Byte;
		desc.p = pixels;

		texture->subImage(desc);
	}
}

MaterialManager::MaterialManager()
	:m_textureIndices(),
	m_textureRgba8(),
	m_textureRgb8(),
	m_materialIndexBuffer(),
	m_materialEntries(),
	m_materialFreeEntries(0),
	m_materialFirstFreeEntry(0),
	m_textureEntries(),
	m_textureFreeEntries(0),
	m_textureFirstFreeEntry(0)
{

}

MaterialManager::~MaterialManager()
{

}

void MaterialManager::createTextures(const vx::uint3 &textureDim)
{
	vx::gl::TextureDescription desc;
	desc.size = textureDim;
	desc.miplevels = 1;
	desc.format = vx::gl::TextureFormat::RGBA8;
	desc.sparse = 1;
	desc.type = vx::gl::TextureType::Texture_2D_Array;

	m_textureRgba8.create(desc);

	desc.format = vx::gl::TextureFormat::RGB8;
	m_textureRgb8.create(desc);
}

void MaterialManager::createBuffer(u32 maxInstances)
{
	vx::gl::BufferDescription desc;
	desc.bufferType = vx::gl::BufferType::Shader_Storage_Buffer;
	desc.immutable = 1;
	desc.pData = nullptr;
	desc.flags = vx::gl::BufferStorageFlags::Write;
	desc.size = sizeof(u32) * maxInstances;

	m_materialIndexBuffer.create(desc);
}

void MaterialManager::initialize(const vx::uint3 &textureDim, u32 maxInstances)
{
	auto textureCount = textureDim.z;
	m_textureEntries = vx::make_unique<std::pair<u16, u16>[]>(textureCount);
	for (u32 i = 0; i < textureCount; ++i)
	{
		m_textureEntries[i].first = i + 1;
		m_textureEntries[i].second = 0;
	}

	m_materialEntries = vx::make_unique<u32[]>(maxInstances);
	for (u32 i = 0; i < maxInstances; ++i)
	{
		m_materialEntries[i] = i + 1;
	}

	createTextures(textureDim);
	createBuffer(maxInstances);

	m_textureFirstFreeEntry = 0;
	m_materialFirstFreeEntry = 0;

	m_textureFreeEntries = textureCount;
	m_materialFreeEntries = maxInstances;
}

void MaterialManager::shutdown()
{
	m_textureRgba8.destroy();
	m_textureRgb8.destroy();
	m_materialIndexBuffer.destroy();
}

bool MaterialManager::addTexture(const vx::StringID &sid, FileAspectInterface* fileAspect, u32* index)
{
	if (m_textureFreeEntries == 0)
		return false;

	auto currentIndex = m_textureFirstFreeEntry;
	m_textureFirstFreeEntry = m_textureEntries[currentIndex].first;
	auto isCommitted = m_textureEntries[currentIndex].second;

	auto texture = fileAspect->getTexture(sid);
	auto &face = texture->getFace(0);
	auto comp = texture->getComponents();
	auto dim = face.getDimension();

	m_textureEntries[currentIndex].second = 1;

	if (comp == 4)
	{
		MaterialManagerCpp::uploadTextureData(face.getPixels(), dim.x, dim.y, currentIndex, isCommitted, &m_textureRgba8);
	}
	else if (comp == 3)
	{
		MaterialManagerCpp::uploadTextureData(face.getPixels(), dim.x, dim.y, currentIndex, isCommitted, &m_textureRgb8);
	}
	else
	{
		VX_ASSERT(false);
	}

	m_textureIndices.insert(sid, currentIndex);
	*index = currentIndex;

	--m_textureFreeEntries;

	return true;
}

bool MaterialManager::getTextureIndex(const vx::StringID &sid, FileAspectInterface* fileAspect, u32* index)
{
	bool result = false;

	auto it = m_textureIndices.find(sid);
	if (it == m_textureIndices.end())
	{
		result = addTexture(sid, fileAspect, index);
	}
	else
	{
		*index = (*it);
		result = true;
	}

	return result;
}

bool MaterialManager::addMaterial(const Material &material, FileAspectInterface* fileAspect, u32* index)
{
	if (m_materialFreeEntries == 0)
		return false;

	auto diffuseSid = material.m_textureSid[0];
	auto normalSid = material.m_textureSid[1];
	auto surfaceSid = material.m_textureSid[2];

	u32 indexDiffuse = 0;
	if (!getTextureIndex(diffuseSid, fileAspect, &indexDiffuse))
	{
		return false;
	}

	u32 indexNormal = 0;
	if (!getTextureIndex(normalSid, fileAspect, &indexNormal))
	{
		return false;
	}

	u32 indexSurface = 0;
	if (!getTextureIndex(surfaceSid, fileAspect, &indexDiffuse))
	{
		return false;
	}

	u32 packedTextureIndices = indexDiffuse | (indexNormal << 8) | (indexSurface << 16);

	u32 materialIndex = m_materialFirstFreeEntry;
	m_materialFirstFreeEntry = m_materialEntries[m_materialFirstFreeEntry];

	--m_materialFreeEntries;

	auto sid = material.getSid();
	m_materialIndices.insert(sid, materialIndex);

	*index = materialIndex;

	auto mappedBuffer = m_materialIndexBuffer.mapRange<u32>(sizeof(u32) * materialIndex, sizeof(u32), vx::gl::MapRange::Write);
	*mappedBuffer = packedTextureIndices;
	mappedBuffer.unmap();

	return true;
}

bool MaterialManager::getMaterialIndex(const Material &material, FileAspectInterface* fileAspect, u32* index)
{
	bool result = false;

	auto materialSid = material.getSid();
	auto it = m_materialIndices.find(materialSid);
	if (it == m_materialIndices.end())
	{
		result = addMaterial(material, fileAspect, index);
	}
	else
	{
		*index = (*it);
		result = true;
	}

	return result;
}