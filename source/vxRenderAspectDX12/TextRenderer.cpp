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

#include "TextRenderer.h"
#include <vxLib/File/FileHandle.h>
#include "Segment.h"
#include "CommandList.h"
#include <vxEngineLib/Logfile.h>

namespace Graphics
{
	/*struct TextRenderer::Entry
	{
		char m_text[48];
		vx::float4 m_color;
		vx::float2 m_position;
		u32 m_size;
	};

	struct TextRenderer::TextVertex
	{
		vx::float3 position;
		vx::float3 uv;
		vx::float4 color;
	};

	TextRenderer::TextRenderer()
		:m_vboId(0),
		m_cmdId(0),
		m_entries(),
		m_vertices(),
		m_entryCount(0),
		m_size(0),
		m_capacity(0),
		m_texureIndex(0),
		m_font(nullptr)
	{

	}

	TextRenderer::~TextRenderer()
	{

	}

	void TextRenderer::createVertexBuffer()
	{
		const auto verticesPerCharacter = 4u;
		auto totalVertexCount = verticesPerCharacter * m_capacity;

		vx::gl::BufferDescription desc{};
		desc.bufferType = vx::gl::BufferType::Array_Buffer;
		desc.flags = vx::gl::BufferStorageFlags::Write | vx::gl::BufferStorageFlags::Dynamic_Storage;
		desc.immutable = 1;
		desc.size = totalVertexCount * sizeof(TextVertex);

		auto vboSid = s_objectManager->createBuffer("textVbo", desc);
		auto vbo = s_objectManager->getBuffer(vboSid);

		m_vboId = vbo->getId();

		m_vertices = vx::make_unique<TextVertex[]>(totalVertexCount);
	}

	void TextRenderer::createIndexBuffer()
	{
		const auto indicesPerCharacter = 6u;
		auto indexCount = indicesPerCharacter * m_capacity;


		auto incides = std::make_unique<u32[]>(indexCount);
		for (u32 i = 0, j = 0; i < indexCount; i += 6, j += 4)
		{
			incides[i] = j;
			incides[i + 1] = j + 1;
			incides[i + 2] = j + 2;

			incides[i + 3] = j + 2;
			incides[i + 4] = j + 3;
			incides[i + 5] = j;
		}

		vx::gl::BufferDescription desc{};
		desc.bufferType = vx::gl::BufferType::Element_Array_Buffer;
		desc.flags = 0;
		desc.immutable = 1;
		desc.size = sizeof(u32) * indexCount;
		desc.pData = incides.get();

		auto iboSid = s_objectManager->createBuffer("textIbo", desc);
	}

	void TextRenderer::createVao()
	{
		auto vaoSid = s_objectManager->createVertexArray("textVao");
		auto vao = s_objectManager->getVertexArray(vaoSid);

		vao->enableArrayAttrib(0);
		vao->enableArrayAttrib(1);
		vao->enableArrayAttrib(2);

		vao->arrayAttribFormatF(0, 3, 0, 0);
		vao->arrayAttribFormatF(1, 3, 0, sizeof(vx::float3));
		vao->arrayAttribFormatF(2, 4, 0, sizeof(vx::float3) * 2);

		vao->arrayAttribBinding(0, 0);
		vao->arrayAttribBinding(1, 0);
		vao->arrayAttribBinding(2, 0);

		auto vbo = s_objectManager->getBuffer("textVbo");
		auto ibo = s_objectManager->getBuffer("textIbo");

		vao->bindVertexBuffer(*vbo, 0, 0, sizeof(TextVertex));
		vao->bindIndexBuffer(*ibo);
	}

	void TextRenderer::createCmdBuffer()
	{
		vx::gl::DrawElementsIndirectCommand cmd{};
		cmd.instanceCount = 1;

		vx::gl::BufferDescription desc{};
		desc.bufferType = vx::gl::BufferType::Draw_Indirect_Buffer;
		desc.flags = vx::gl::BufferStorageFlags::Write | vx::gl::BufferStorageFlags::Dynamic_Storage;
		desc.immutable = 1;
		desc.size = sizeof(vx::gl::DrawElementsIndirectCommand);
		desc.pData = &cmd;

		auto cmdSid = s_objectManager->createBuffer("textCmd", desc);
		auto cmdBuffer = s_objectManager->getBuffer(cmdSid);

		m_cmdId = cmdBuffer->getId();
	}

	bool TextRenderer::initialize(vx::StackAllocator* scratchAllocator, Logfile* errorlog, const void* p)
	{
		auto desc = (TextRendererDesc*)p;

		m_entries = (Entry*)desc->allocator->allocate(sizeof(Entry) * s_maxEntryCount, __alignof(Entry));
		m_entryCount = 0;

		std::string error;
		if (!s_shaderManager->loadPipeline(vx::FileHandle("text.pipe"), "text.pipe", scratchAllocator, &error))
		{
			errorlog->append(error.c_str(), error.size());
			error.clear();
			return false;
		}

		m_font = desc->font;
		m_capacity = desc->maxCharacters;

		createVertexBuffer();
		createIndexBuffer();
		createVao();
		createCmdBuffer();

		return true;
	}

	void TextRenderer::shutdown()
	{
	}

	void TextRenderer::pushEntry(const char(&text)[48], u32 size, const vx::float2 &topLeftPosition, const vx::float3 &color)
	{
		VX_ASSERT(m_entryCount < s_maxEntryCount);

		auto &entry = m_entries[m_entryCount++];

		strncpy(entry.m_text, text, 48);
		entry.m_position = topLeftPosition;
		entry.m_color = vx::float4(color, 1.0f);
		entry.m_size = size;

		m_size += size;
	}

	void TextRenderer::update()
	{
		if (m_size == 0)
			return;

		updateVertexBuffer();
	}

	void TextRenderer::updateVertexBuffer()
	{
		u32 offset = 0;

		f32 textureSlice = m_font->getTextureSlice();
		auto textureSize = m_font->getTextureDim();

		vx::float4a invTextureSize;
		invTextureSize.x = 1.0f / textureSize;

		auto vInvTexSize = _mm_shuffle_ps(invTextureSize.v, invTextureSize.v, _MM_SHUFFLE(0, 0, 0, 0));

		auto entryCount = m_entryCount;
		auto entries = m_entries;
		for (u32 i = 0;i < entryCount; ++i)
		{
			writeEntryToVertexBuffer(vInvTexSize, entries[i], &offset, textureSize, textureSlice);
		}

		if (offset != 0)
		{
			auto indexCount = offset * 6;
			auto vertexCount = offset * 6;

			glNamedBufferSubData(m_vboId, 0, sizeof(TextVertex) * vertexCount, m_vertices.get());

			glNamedBufferSubData(m_cmdId, 0, sizeof(u32), &indexCount);
		}

		m_size = 0;
		//m_entries.clear();
		m_entryCount = 0;
	}

	void TextRenderer::writeEntryToVertexBuffer(const __m128 invTextureSize, const Entry &entry, u32* offset, u32 textureSize, u32 textureSlice)
	{
		auto entryText = entry.m_text;
		auto entryTextSize = entry.m_size;
		auto entryOrigin = entry.m_position;
		auto entryColor = vx::loadFloat4(entry.m_color);

		auto currentSize = m_size;

		if (currentSize + entryTextSize >= m_capacity)
			return;

		u32 vertexOffset = (*offset) * 4;
		*offset += entryTextSize;

		auto vertices = &m_vertices[vertexOffset];
		u32 vertexIndex = 0;

		const f32 scale = 0.25f;
		const __m128 vScale = { scale, scale, 0, 0 };
		const __m128 tmp = { -1.0f, 0, 0, 0 };

		auto cursorPosition = entryOrigin;
		for (u32 i = 0; i < entryTextSize; ++i)
		{
			char ascii_code = entryText[i];

			if (ascii_code == '\n')
			{
				cursorPosition.y -= 53.0f * scale;
				cursorPosition.x = entryOrigin.x;
			}
			else
			{
				__m128 vCursorPos = { cursorPosition.x, cursorPosition.y, 0.0f, 0.0f };

				auto pAtlasEntry = m_font->getAtlasEntry(ascii_code);
				vx::float4a texRect(pAtlasEntry->x, textureSize - pAtlasEntry->y - pAtlasEntry->height, pAtlasEntry->width, pAtlasEntry->height);

				__m128 vAtlasPos = { pAtlasEntry->offsetX, pAtlasEntry->offsetY, 0.0f, 0.0f };

				__m128 vEntrySize = { texRect.z, -texRect.w, 0.0f, 0.0f };

				__m128 vCurrentPosition = _mm_div_ps(vEntrySize, vx::g_VXTwo);
				vCurrentPosition = _mm_add_ps(vCurrentPosition, vAtlasPos);
				vCurrentPosition = vx::fma(vCurrentPosition, vScale, vCursorPos);
				vCurrentPosition = _mm_movelh_ps(vCurrentPosition, tmp);

				__m128 vSource = _mm_mul_ps(texRect, invTextureSize);
				__m128 vSourceSize = _mm_shuffle_ps(vSource, vSource, _MM_SHUFFLE(3, 2, 3, 2));

				static const __m128 texOffsets[4] =
				{
					{ 0, 0, 0, 0 },
					{ 1, 0, 0, 0 },
					{ 1, 1, 0, 0 },
					{ 0, 1, 0, 0 }
				};

				static const __m128 posOffsets[4] =
				{
					{ -0.5f, -0.5f, 0, 0 },
					{ 0.5f, -0.5f, 0, 0 },
					{ 0.5f, 0.5f, 0, 0 },
					{ -0.5f, 0.5f, 0, 0 }
				};

				vEntrySize = _mm_shuffle_ps(texRect, texRect, _MM_SHUFFLE(3, 2, 3, 2));

				for (u32 k = 0; k < 4; ++k)
				{
					u32 index = vertexIndex + k;

					auto pos = _mm_mul_ps(posOffsets[k], vEntrySize);
					pos = vx::fma(pos, vScale, vCurrentPosition);
					_mm_storeu_ps(&vertices[index].position.x, pos);

					__m128 uv = vx::fma(texOffsets[k], vSourceSize, vSource);
					_mm_storeu_ps(&vertices[index].uv.x, uv);
					vertices[index].uv.z = textureSlice;

					_mm_storeu_ps(&vertices[index].color.x, entryColor);
				}
				vertexIndex += 4;

				cursorPosition.x = fmaf(pAtlasEntry->advanceX, scale, cursorPosition.x);
			}
		}
	}

	void TextRenderer::getCommandList(CommandList* cmdList)
	{
		auto pipeline = s_shaderManager->getPipeline("text.pipe");
		auto vao = s_objectManager->getVertexArray("textVao");
		auto fsId = pipeline->getFragmentShader();
		auto cmdBuffer = s_objectManager->getBuffer("textCmd");

		StateDescription stateDesc =
		{
			0,
			vao->getId(),
			pipeline->getId(),
			cmdBuffer->getId(),
			0,
			false,
			true,
			false,
			true,
			{ 1, 1, 1, 1 },
			1
		};

		State state;
		state.set(stateDesc);

		BlendEquationCommand blendEquCmdM;
		blendEquCmdM.set(GL_FUNC_ADD);

		BlendFuncCommand blendFuncCmd;
		blendFuncCmd.set(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		ProgramUniformCommand uniformCmd;
		uniformCmd.set(fsId, 0, 1, vx::gl::DataType::Unsigned_Int);

		DrawElementsIndirectCommand drawCmd;
		drawCmd.set(GL_TRIANGLES, GL_UNSIGNED_INT);

		Segment drawText;
		drawText.setState(state);
		drawText.pushCommand(blendEquCmdM);
		drawText.pushCommand(blendFuncCmd);
		drawText.pushCommand(uniformCmd, reinterpret_cast<u8*>(&m_texureIndex));
		drawText.pushCommand(drawCmd);

		cmdList->pushSegment(drawText, "drawText");
	}

	void TextRenderer::clearData()
	{

	}

	void TextRenderer::bindBuffers()
	{

	}*/
}