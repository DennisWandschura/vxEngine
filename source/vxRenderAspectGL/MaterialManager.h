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
class FileAspectInterface;

namespace gl
{
	class ObjectManager;
}

#include <vxLib/Container/sorted_vector.h>
#include <vxGL/Texture.h>
#include <vxLib/StringID.h>
#include <memory>
#include <vxgl/Buffer.h>

class MaterialManager
{
	vx::sorted_vector<vx::StringID, u32> m_textureIndices;
	vx::sorted_vector<vx::StringID, u32> m_materialIndices;
	vx::gl::Texture m_textureRgba8;
	vx::gl::Texture m_textureRgb8;
	vx::gl::Buffer m_materialIndexBuffer;
	std::unique_ptr<u32[]> m_materialEntries;
	u32 m_materialFreeEntries;
	u32 m_materialFirstFreeEntry;
	std::unique_ptr<std::pair<u16, u16>[]> m_textureEntries;
	u32 m_textureFreeEntries;
	u32 m_textureFirstFreeEntry;

	void createTextures(const vx::uint3 &textureDim);
	void createBuffer(u32 maxInstances);

	bool addTexture(const vx::StringID &sid,FileAspectInterface* fileAspect, u32* index);
	bool getTextureIndex(const vx::StringID &sid, FileAspectInterface* fileAspect, u32* index);

	bool addMaterial(const Material &material, FileAspectInterface* fileAspect, u32* index);

public:
	MaterialManager();
	~MaterialManager();

	void initialize(const vx::uint3 &textureDim, u32 maxInstances);
	void shutdown();

	bool getMaterialIndex(const Material &material, FileAspectInterface* fileAspect, u32* index);
};