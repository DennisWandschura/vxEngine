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

#include <vxLib/types.h>

class Material;

namespace Graphics
{
	class Texture;
}

template<typename T>
class ResourceManager;

namespace vx
{
	template<typename K, typename T,typename C>
	class sorted_array;

	struct StringID;

	class FileEntry;
}

#include <vxEngineLib/FileEntry.h>
#include <vector>

struct MaterialFactoryLoadDescription
{
	const char *fileNameWithPath;
	const vx::sorted_array<vx::StringID, const Graphics::Texture*, std::less<vx::StringID>>* textureFiles;
	std::vector<vx::FileEntry>* missingFiles;
	Material* material;
};

struct MissingTextureFile
{
	vx::FileEntry fileEntry;
	bool srgb;
};

struct MaterialFactoryLoadDescNew
{
	const char *fileNameWithPath;
	const ResourceManager<Graphics::Texture>* m_textureManager;
	MissingTextureFile* missingFiles;
	u32* missingFilesCount;
	Material* material;
};

class MaterialFactory
{
public:
	static bool load(const MaterialFactoryLoadDescription &desc);
	static bool load(const MaterialFactoryLoadDescNew &desc);
};