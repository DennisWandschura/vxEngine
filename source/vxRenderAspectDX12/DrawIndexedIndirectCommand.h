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

struct ID3D12CommandSignature;
struct ID3D12GraphicsCommandList;
struct D3D12_DRAW_INDEXED_ARGUMENTS;
class UploadManager;

namespace d3d
{
	class ResourceManager;
}

#include "d3d.h"

class DrawIndexedIndirectCommand
{
	d3d::Object<ID3D12CommandSignature> m_commandSignature;
	ID3D12Resource* m_commmandBuffer;
	u32 m_count;
	u32 m_countOffset;
	u32 m_maxCount;

	bool createSignature(ID3D12Device* device);

public:
	DrawIndexedIndirectCommand();
	DrawIndexedIndirectCommand(const DrawIndexedIndirectCommand&) = delete;
	DrawIndexedIndirectCommand(DrawIndexedIndirectCommand &&rhs);
	~DrawIndexedIndirectCommand();

	void getRequiredMemory(u32 maxCount, u64* bufferHeapSize);

	bool create(const wchar_t* id, u32 maxCount, d3d::ResourceManager* resourceManager, ID3D12Device* device);
	void destroy();

	void uploadDrawCommand(u32 index, const D3D12_DRAW_INDEXED_ARGUMENTS &cmd, UploadManager* uploadManager);
	void setCount(u32 count, UploadManager* uploadManager);

	void incrementCount(UploadManager* uploadManager)
	{
		setCount(m_count + 1, uploadManager);
	}

	void draw(ID3D12GraphicsCommandList* cmdList);

	u32 getCount() const { return m_count; }
	u32 getMaxCount() const { return m_maxCount; }
};