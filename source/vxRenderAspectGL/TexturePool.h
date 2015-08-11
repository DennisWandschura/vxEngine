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

class ResourceAspectInterface;

namespace Graphics
{
	class Texture;
}

#include <vxLib/Container/sorted_vector.h>
#include <vxGL/Texture.h>
#include <memory>
#include <vxLib/StringID.h>

class TexturePool
{
	vx::sorted_vector<vx::StringID, u32> m_indices;
	std::unique_ptr<std::pair<u16, u16>[]> m_entries;
	u32 m_firstFreeEntry;
	u32 m_freeEntries;
	vx::gl::Texture m_texture;

	bool addTexture(const vx::StringID &sid, ResourceAspectInterface* resourceAspect, u32* index);
	bool addTexture(const vx::StringID &sid, const Graphics::Texture &texture, u32* index);

public:
	TexturePool();
	~TexturePool();

	void initialize(const vx::uint3 &textureDim, vx::gl::TextureFormat format);
	void shutdown();

	bool getTextureIndex(const vx::StringID &sid, ResourceAspectInterface* resourceAspect, u32* index);
	bool getTextureIndex(const vx::StringID &sid, const Graphics::Texture &texture, u32* index);

	u32 getTextureId(const vx::StringID &sid) const;

	u64 getTextureHandle() const;
};
