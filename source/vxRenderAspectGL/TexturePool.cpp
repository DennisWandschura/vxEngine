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

#include "TexturePool.h"
#include <vxEngineLib/ResourceAspectInterface.h>
#include <vxEngineLib/Graphics/Texture.h>

namespace TexturePoolCpp
{
	void uploadTextureData(const u8* pixels, u32 w, u32 h, u32 index, u32 dataSize, u8 isCommitted, vx::gl::Texture* texture)
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

		/*vx::gl::TextureSubImageDescription desc;
		desc.size = vx::uint3(w, h, 1);
		desc.miplevel = 0;
		desc.offset = vx::uint3(0, 0, index);
		desc.dataType = vx::gl::DataType::Unsigned_Byte;
		desc.p = pixels;

		texture->subImage(desc);*/
		vx::gl::TextureCompressedSubImageDescription desc;
		desc.size = vx::uint3(w, h, 1);
		desc.offset = vx::uint3(0, 0, index);
		desc.miplevel = 0;
		desc.p = pixels;
		desc.dataSize = dataSize;
		texture->subImageCompressed(desc);
	}
}

TexturePool::TexturePool()
	:m_indices(),
	m_entries(),
	m_firstFreeEntry(0),
	m_freeEntries(0),
	m_texture()
{

}

TexturePool::~TexturePool()
{

}

void TexturePool::initialize(const vx::uint3 &textureDim, vx::gl::TextureFormat format)
{
	auto textureCount = textureDim.z;
	m_indices.reserve(textureCount);
	m_entries = std::make_unique<std::pair<u16, u16>[]>(textureCount);
	for (u32 i = 0; i < textureCount; ++i)
	{
		m_entries[i].first = i + 1;
		m_entries[i].second = 0;
	}

	m_firstFreeEntry = 0;
	m_freeEntries = textureCount;

	vx::gl::TextureDescription desc;
	desc.size = textureDim;
	desc.miplevels = 1;
	desc.format = format;
	desc.sparse = 1;
	desc.type = vx::gl::TextureType::Texture_2D_Array;

	m_texture.create(desc);
	m_texture.makeTextureResident();
}

void TexturePool::shutdown()
{
	m_texture.destroy();
}

bool TexturePool::addTexture(const vx::StringID &sid, ResourceAspectInterface* resourceAspect, u32* index)
{
	auto texture = resourceAspect->getTexture(sid);

	return addTexture(sid, *texture, index);
}

bool TexturePool::addTexture(const vx::StringID &sid, const Graphics::Texture &texture, u32* index)
{
	if (m_freeEntries == 0)
		return false;

	auto currentIndex = m_firstFreeEntry;
	m_firstFreeEntry = m_entries[currentIndex].first;
	auto isCommitted = m_entries[currentIndex].second;

	auto &face = texture.getFace(0);
	auto dim = face.getDimension();
	auto dataSize = face.getSize();

	m_entries[currentIndex].second = 1;

	TexturePoolCpp::uploadTextureData(face.getPixels(), dim.x, dim.y, currentIndex, dataSize, isCommitted, &m_texture);

	m_indices.insert(sid, currentIndex);
	*index = currentIndex;

	--m_freeEntries;

	return true;
}

bool TexturePool::getTextureIndex(const vx::StringID &sid, ResourceAspectInterface* resourceAspect, u32* index)
{
	bool result = false;

	auto it = m_indices.find(sid);
	if (it == m_indices.end())
	{
		result = addTexture(sid, resourceAspect, index);
	}
	else
	{
		*index = (*it);
		result = true;
	}

	return result;
}

bool TexturePool::getTextureIndex(const vx::StringID &sid, const Graphics::Texture &texture, u32* index)
{
	bool result = false;

	auto it = m_indices.find(sid);
	if (it == m_indices.end())
	{
		result = addTexture(sid, texture, index);
	}
	else
	{
		*index = (*it);
		result = true;
	}

	return result;
}

u32 TexturePool::getTextureId(const vx::StringID &sid) const
{
	u32 id = 0;

	auto it = m_indices.find(sid);
	if (it != m_indices.end())
	{
		id = m_texture.getId();
	}

	return id;
}

u64 TexturePool::getTextureHandle() const
{
	return m_texture.getTextureHandle();
}