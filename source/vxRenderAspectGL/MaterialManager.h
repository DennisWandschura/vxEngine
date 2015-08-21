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

class Material;

namespace gl
{
	class ObjectManager;
}

#include "TexturePool.h"
#include <vxgl/Buffer.h>

class MaterialManager
{
	TexturePool m_poolSrgba;
	TexturePool m_poolRgba;

	vx::sorted_vector<vx::StringID, u32> m_materialIndices;
	std::unique_ptr<u32[]> m_materialEntries;
	u32 m_materialFreeEntries;
	u32 m_materialFirstFreeEntry;
	gl::ObjectManager* m_objectManager;

	void createBuffer(u32 maxInstances);

	bool addMaterial(const Material &material, ResourceAspectInterface* resourceAspect, u32* index);
	bool addMaterial(const vx::StringID &materialSid, ResourceAspectInterface* resourceAspect, u32* index);

public:
	MaterialManager();
	~MaterialManager();

	void initialize(const vx::uint3 &textureDim, u32 maxInstances, gl::ObjectManager* objectManager);
	void shutdown();

	bool getMaterialIndex(const Material &material, ResourceAspectInterface* resourceAspect, u32* index);
	bool getMaterialIndex(const vx::StringID &materialSid, ResourceAspectInterface* resourceAspect, u32* index);

	bool getTextureIndex(const vx::StringID &sid, const Graphics::Texture &texture, u32* index);

	u32 getTextureId(const vx::StringID &sid) const;

	u32 setTextTexture(const Graphics::Texture &texture);
};