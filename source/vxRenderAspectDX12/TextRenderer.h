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

struct ID3D12Device;
class UploadManager;

namespace d3d
{
	class ResourceManager;
}

#include <vxLib/math/Vector.h>
#include <vxLib/memory.h>
#include <vector>
#include <vxLib/Allocator/StackAllocator.h>

class RenderPassText;

namespace Graphics
{
	class Font;

	struct TextRendererDesc
	{
		vx::StackAllocator* allocator;
		ID3D12Device* device;
		d3d::ResourceManager* resourceManager;
		UploadManager* uploadManager;
		RenderPassText* m_renderPassText;
		u32 maxCharacters;
	};

	class TextRenderer
	{
		static const auto s_maxEntryCount = 256u;
		struct Entry;
		struct TextVertex;

		u32 m_vboId;
		u32 m_cmdId;
		Entry* m_entries;
		std::unique_ptr<TextVertex[]> m_vertices;
		RenderPassText* m_renderPassText;
		u32 m_entryCount;
		u32 m_size;
		u32 m_capacity;
		u32 m_texureIndex;
		const Font* m_font;

		//void createCmdBuffer();

		void updateVertexBuffer(UploadManager* uploadManager, d3d::ResourceManager* resourceManager);
		void writeEntryToVertexBuffer(const __m128 invTextureSize, const Entry &entry, u32* offset, u32 textureSize);

	public:
		TextRenderer();
		~TextRenderer();

		bool createData(ID3D12Device* device, d3d::ResourceManager* resourceManager, UploadManager* uploadManager, u32 maxCharacters);

		void getRequiredMemory(u64* bufferSize, u32* bufferCount, u64* textureSize, u32*textureCount, ID3D12Device* device, u32 maxCharacters);

		bool initialize(vx::StackAllocator* scratchAllocator, const void* p);
		void shutdown();

		void pushEntry(const char(&text)[48], u32 strSize, const vx::float2 &topLeftPosition, const vx::float3 &color);

		void setFont(const Font* font) { m_font = font; }

		void update(UploadManager* uploadManager, d3d::ResourceManager* resourceManager, bool upload);
	};
}