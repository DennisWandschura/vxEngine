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
#include <vxEngineLib/Reference.h>
#include "GpuStructs.h"

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
	:m_poolSrgba(),
	m_poolRgb(),
	m_materialEntries(),
	m_materialFreeEntries(0),
	m_materialFirstFreeEntry(0),
	m_objectManager(nullptr)
{

}

MaterialManager::~MaterialManager()
{

}

void MaterialManager::createBuffer(u32 maxInstances)
{
	vx::gl::BufferDescription desc;
	desc.bufferType = vx::gl::BufferType::Shader_Storage_Buffer;
	desc.immutable = 1;
	desc.pData = nullptr;
	desc.flags = vx::gl::BufferStorageFlags::Write;
	desc.size = sizeof(u32) * maxInstances;

	m_objectManager->createBuffer("materialBlockBuffer", desc);
}

void MaterialManager::initialize(const vx::uint3 &textureDim, u32 maxInstances, gl::ObjectManager* objectManager)
{
	m_objectManager = objectManager;

	m_materialEntries = vx::make_unique<u32[]>(maxInstances);
	for (u32 i = 0; i < maxInstances; ++i)
	{
		m_materialEntries[i] = i + 1;
	}

	m_poolSrgba.initialize(textureDim, vx::gl::TextureFormat::SRGBA8);
	m_poolRgb.initialize(textureDim, vx::gl::TextureFormat::RGB8);

	auto handle0 = m_poolSrgba.getTextureHandle();
	auto handle1 = m_poolRgb.getTextureHandle();

	auto uniformTextureBuffer = objectManager->getBuffer("UniformTextureBuffer");
	auto mappedBuffer = uniformTextureBuffer->map<Gpu::UniformTextureBufferBlock>(vx::gl::Map::Write_Only);
	mappedBuffer->u_srgb = handle0;
	mappedBuffer->u_rgb = handle1;
	mappedBuffer.unmap();

	createBuffer(maxInstances);

	m_materialFirstFreeEntry = 0;

	m_materialFreeEntries = maxInstances;
}

void MaterialManager::shutdown()
{
	m_poolSrgba.shutdown();
	m_poolRgb.shutdown();
}

bool MaterialManager::addMaterial(const vx::StringID &materialSid, FileAspectInterface* fileAspect, u32* index)
{
	Reference<Material> material = fileAspect->getMaterial(materialSid);
	auto ptr = material.get();
	if (ptr == nullptr)
		return false;

	return addMaterial(*ptr, fileAspect, index);
}

bool MaterialManager::addMaterial(const Material &material, FileAspectInterface* fileAspect, u32* index)
{
	if (m_materialFreeEntries == 0)
		return false;

	auto diffuseSid = material.m_textureSid[0];
	auto normalSid = material.m_textureSid[1];
	auto surfaceSid = material.m_textureSid[2];

	auto comp = fileAspect->getTexture(diffuseSid)->getComponents();
	VX_ASSERT(comp == 4);
	u32 indexDiffuse = 0;
	if (!m_poolSrgba.getTextureIndex(diffuseSid, fileAspect, &indexDiffuse))
	{
		return false;
	}

	comp = fileAspect->getTexture(normalSid)->getComponents();
	VX_ASSERT(comp == 3);
	u32 indexNormal = 0;
	if (!m_poolRgb.getTextureIndex(normalSid, fileAspect, &indexNormal))
	{
		return false;
	}

	comp = fileAspect->getTexture(surfaceSid)->getComponents();
	VX_ASSERT(comp == 4);

	u32 indexSurface = 0;
	if (!m_poolSrgba.getTextureIndex(surfaceSid, fileAspect, &indexSurface))
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

	auto materialBuffer = m_objectManager->getBuffer("materialBlockBuffer");
	auto mappedBuffer = materialBuffer->mapRange<u32>(sizeof(u32) * materialIndex, sizeof(u32), vx::gl::MapRange::Write);
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

bool MaterialManager::getMaterialIndex(const vx::StringID &materialSid, FileAspectInterface* fileAspect, u32* index)
{
	bool result = false;

	auto it = m_materialIndices.find(materialSid);
	if (it == m_materialIndices.end())
	{
		result = addMaterial(materialSid, fileAspect, index);
	}
	else
	{
		*index = (*it);
		result = true;
	}

	return result;
}

bool MaterialManager::getTextureIndex(const vx::StringID &sid, const Graphics::Texture &texture, u32* index)
{
	auto comp = texture.getComponents();
	bool result = false;
	if (comp == 4)
	{
		result = m_poolSrgba.getTextureIndex(sid, texture, index);
	}
	else if (comp == 3)
	{
		result = m_poolRgb.getTextureIndex(sid, texture, index);
	}

	return result;
}

u32 MaterialManager::getTextureId(const vx::StringID &sid) const
{
	auto id = m_poolSrgba.getTextureId(sid);
	if (id == 0)
	{
		id = m_poolRgb.getTextureId(sid);
	}

	return id;
}