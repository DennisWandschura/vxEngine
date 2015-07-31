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

class Font;

namespace vx
{
	namespace gl
	{
		class ShaderManager;
	}
}

#include "Renderer.h"
#include <vxLib/math/Vector.h>
#include <vxLib/memory.h>
#include <vector>
#include <vxLib/Allocator/StackAllocator.h>

namespace Graphics
{
	struct TextRendererDesc
	{
		const Font* font;
		vx::StackAllocator* allocator;
		u32 maxCharacters;
		u32 textureIndex;
	};

	class TextRenderer : public Renderer
	{
		static const auto s_maxEntryCount = 256u;
		struct Entry;
		struct TextVertex;

		u32 m_vboId;
		u32 m_cmdId;
		Entry* m_entries;
		std::unique_ptr<TextVertex[]> m_vertices;
		u32 m_entryCount;
		u32 m_size;
		u32 m_capacity;
		u32 m_texureIndex;
		const Font* m_font;

		void createVertexBuffer();
		void createIndexBuffer();
		void createVao();
		void createCmdBuffer();

		void updateVertexBuffer();
		void writeEntryToVertexBuffer(const __m128 invTextureSize, const Entry &entry, u32* offset, u32 textureSize, u32 textureSlice);

	public:
		TextRenderer();
		~TextRenderer();

		bool initialize(vx::StackAllocator* scratchAllocator, const void* p) override;
		void shutdown() override;

		void pushEntry(const char(&text)[48], u32 size, const vx::float2 &topLeftPosition, const vx::float3 &color);

		void update();

		void getCommandList(CommandList* cmdList) override;

		void clearData() override;
		void bindBuffers() override;
	};
}