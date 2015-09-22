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
#include <vxEngineLib/Logfile.h>
#include "ResourceDesc.h"
#include "d3d.h"
#include <vxEngineLib/Graphics/Font.h>
#include <vxEngineLib/Graphics/Texture.h>
#include "UploadManager.h"
#include "ResourceManager.h"
#include "RenderPassText.h"
#include "ResourceView.h"

namespace Graphics
{
	struct TextRenderer::Entry
	{
		char m_text[48];
		vx::float4 m_color;
		vx::float2 m_position;
		u32 m_size;
	};

	struct TextRenderer::TextVertex
	{
		vx::float3 position;
		vx::float2 uv;
		vx::float4 color;
	};

	TextRenderer::TextRenderer()
		:m_vboId(0),
		m_cmdId(0),
		m_entries(),
		m_vertices(),
		m_renderPassText(nullptr),
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

	void TextRenderer::getRequiredMemory(u64* bufferSize, u64* textureSize, ID3D12Device* device, u32 maxCharacters)
	{
		const auto verticesPerCharacter = 4u;
		auto totalVertexCount = verticesPerCharacter * maxCharacters;

		const auto indicesPerCharacter = 6u;
		auto indexCount = indicesPerCharacter * maxCharacters;

		auto vertexSizeInBytes = totalVertexCount * sizeof(TextVertex);
		auto indexSizeInBytes = indexCount * sizeof(u32);

		auto fontTexResDesc = d3d::ResourceDesc::getDescTexture2D(vx::uint2(1024, 1024), DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, D3D12_RESOURCE_FLAG_NONE);
		auto fontTexAllocInfo = device->GetResourceAllocationInfo(1, 1,&fontTexResDesc);

		*textureSize += fontTexAllocInfo.SizeInBytes;
		*bufferSize += d3d::getAlignedSize(vertexSizeInBytes, 64llu KBYTE) + d3d::getAlignedSize(indexSizeInBytes, 64llu KBYTE);
	}

	bool TextRenderer::createData(ID3D12Device* device, d3d::ResourceManager* resourceManager, UploadManager* uploadManager, u32 maxCharacters)
	{
		auto fontTexResDesc = d3d::ResourceDesc::getDescTexture2D(vx::uint2(1024, 1024), DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, D3D12_RESOURCE_FLAG_NONE);
		auto fontTexAllocInfo = device->GetResourceAllocationInfo(1, 1, &fontTexResDesc);

		const auto verticesPerCharacter = 4u;
		auto totalVertexCount = verticesPerCharacter * maxCharacters;

		const auto indicesPerCharacter = 6u;
		auto indexCount = indicesPerCharacter * maxCharacters;

		auto vertexSizeInBytes = d3d::getAlignedSize(totalVertexCount * sizeof(TextVertex), 64llu KBYTE);
		auto indexSizeInBytes = (indexCount * sizeof(u32), 64llu KBYTE);

		CreateResourceDesc texDesc = CreateResourceDesc::createDesc(fontTexAllocInfo.SizeInBytes,&fontTexResDesc, nullptr, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		auto texture = resourceManager->createTexture(L"fontTexture", texDesc);
		if (texture == nullptr)
		{
			return false;
		}

		auto vertexBuffer = resourceManager->createBuffer(L"textVertexBuffer", vertexSizeInBytes, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		if (vertexBuffer == nullptr)
		{
			return false;
		}

		auto indexBuffer = resourceManager->createBuffer(L"textIndexBuffer", indexSizeInBytes, D3D12_RESOURCE_STATE_INDEX_BUFFER);
		if (vertexBuffer == nullptr)
		{
			return false;
		}

		m_vertices = std::make_unique<TextVertex[]>(totalVertexCount);

		return true;
	}

	bool TextRenderer::initialize(vx::StackAllocator* scratchAllocator, const void* p)
	{
		auto desc = (TextRendererDesc*)p;
		m_entries = (Entry*)desc->allocator->allocate(sizeof(Entry) * s_maxEntryCount, __alignof(Entry));
		m_entryCount = 0;

		m_capacity = desc->maxCharacters;

		m_renderPassText = desc->m_renderPassText;

		const auto verticesPerCharacter = 4u;
		auto totalVertexCount = verticesPerCharacter * m_capacity;
		const auto indicesPerCharacter = 6u;
		auto indexCount = indicesPerCharacter * m_capacity;
		auto vertexSizeInBytes = d3d::getAlignedSize(totalVertexCount * sizeof(TextVertex), 64llu KBYTE);
		auto indexSizeInBytes = (indexCount * sizeof(u32), 64llu KBYTE);

		auto textVertexBuffer = desc->resourceManager->getBuffer(L"textVertexBuffer");
		d3d::ResourceView vbv;
		vbv.vbv.BufferLocation = textVertexBuffer->GetGPUVirtualAddress();
		vbv.vbv.SizeInBytes = vertexSizeInBytes;
		vbv.vbv.StrideInBytes = sizeof(TextVertex);
		vbv.type = d3d::ResourceView::Type::VertexBufferView;
		desc->resourceManager->insertResourceView("textVbv", vbv);

		auto textIndexBuffer = desc->resourceManager->getBuffer(L"textIndexBuffer");
		d3d::ResourceView textIbv;
		textIbv.type = d3d::ResourceView::Type::IndexBufferView;
		textIbv.ibv.BufferLocation = textIndexBuffer->GetGPUVirtualAddress();
		textIbv.ibv.Format = DXGI_FORMAT_R32_UINT;
		textIbv.ibv.SizeInBytes = indexSizeInBytes;
		desc->resourceManager->insertResourceView("textIbv", textIbv);

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

		desc->uploadManager->pushUploadBuffer((u8*)incides.get(), textIndexBuffer->get(), 0, sizeof(u32) * indexCount, D3D12_RESOURCE_STATE_INDEX_BUFFER);

		return true;
	}

	void TextRenderer::shutdown()
	{
	}

	void TextRenderer::pushEntry(const char(&text)[48], u32 strSize, const vx::float2 &topLeftPosition, const vx::float3 &color)
	{
		VX_ASSERT(m_entryCount < s_maxEntryCount);

		auto &entry = m_entries[m_entryCount++];

		strncpy(entry.m_text, text, 48);
		entry.m_position = topLeftPosition;
		entry.m_color = vx::float4(color, 1.0f);
		entry.m_size = strSize;

		m_size += strSize;
	}

	void TextRenderer::update(UploadManager* uploadManager, d3d::ResourceManager* resourceManager)
	{
		if (m_size == 0 || m_font == nullptr)
			return;

		updateVertexBuffer(uploadManager, resourceManager);
	}

	void TextRenderer::updateVertexBuffer(UploadManager* uploadManager, d3d::ResourceManager* resourceManager)
	{
		u32 offset = 0;

		auto fontTexture = m_font->getTexture();
		auto textureSize = fontTexture->getFace(0).getDimension().x;

		vx::float4a invTextureSize;
		invTextureSize.x = 1.0f / textureSize;

		auto vInvTexSize = _mm_shuffle_ps(invTextureSize.v, invTextureSize.v, _MM_SHUFFLE(0, 0, 0, 0));

		auto entryCount = m_entryCount;
		auto entries = m_entries;
		for (u32 i = 0; i < entryCount; ++i)
		{
			writeEntryToVertexBuffer(vInvTexSize, entries[i], &offset, textureSize);
		}

		if (offset != 0)
		{
			auto indexCount = offset * 6;
			auto vertexCount = offset * 4;

			auto textVertexBuffer = resourceManager->getBuffer(L"textVertexBuffer");
			uploadManager->pushUploadBuffer((u8*)m_vertices.get(), textVertexBuffer->get(), 0, sizeof(TextVertex) * vertexCount, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

			m_renderPassText->setIndexCount(indexCount);
		}

		m_size = 0;
		m_entryCount = 0;
	}

	void TextRenderer::writeEntryToVertexBuffer(const __m128 invTextureSize, const Entry &entry, u32* offset, u32 textureSize)
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

					_mm_storeu_ps(&vertices[index].color.x, entryColor);
				}
				vertexIndex += 4;

				cursorPosition.x = fmaf(pAtlasEntry->advanceX, scale, cursorPosition.x);
			}
		}
	}

	/*void TextRenderer::getCommandList(CommandList* cmdList)
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
	}*/
}