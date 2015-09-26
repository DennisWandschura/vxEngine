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

#include "CommandList.h"
#include "CommandAllocator.h"
#include <vxEngineLib/Graphics/CommandQueue.h>
#include <vector>

class CopyManager
{
	struct Entry;

	d3d::GraphicsCommandList m_commandListCopy;
	d3d::CommandAllocator m_allocator;
	std::vector<Entry> m_entries;

public:
	CopyManager();
	~CopyManager();

	bool initialize(ID3D12Device* device);
	void destroy();

	void pushCopyBuffer(ID3D12Resource* src, u64 srcOffset, u32 srcStateBefore, u64 size, ID3D12Resource* dst, u64 dstOffset, u32 dstStateBefore);
	void pushCopyTexture(ID3D12Resource* src, u32 srcStateBefore, ID3D12Resource* dst, u32 dstStateBefore);

	void buildCommandList();
	void submitList(Graphics::CommandQueue* queue);
};