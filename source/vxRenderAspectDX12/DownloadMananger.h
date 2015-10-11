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

namespace Graphics
{
	class CommandQueue;
}

struct FrameData;

#include "CommandList.h"
#include "CommandAllocator.h"
#include "Heap.h"
#include "Resource.h"
#include <vector>
#include <vxEngineLib/Event.h>
#include <memory>

class DownloadManager
{
	struct DownloadEntry;
	struct CopyEntry;

	d3d::GraphicsCommandList* m_currentCommandList;
	std::unique_ptr<d3d::GraphicsCommandList[]> m_commandLists;
	u32 m_buildCommandList;
	d3d::Resource m_bufferDownload;
	std::vector<CopyEntry> m_copyEntries;
	std::vector<DownloadEntry> m_entries;
	u32 m_capacity;
	u32 m_size;
	d3d::Heap m_heapDownload;

	bool queueCopyBuffer(d3d::Resource* cpySrc, u32 cpyOffset, u32 size, u8* cpuDst, const Event &evt);

public:
	DownloadManager();
	~DownloadManager();

	bool initialize(ID3D12Device* device, FrameData* frameData, u32 frameCount);
	void shutdown();

	void pushDownloadBuffer(u8* dst, u32 size, d3d::Resource* cpySrc, u32 cpyOffset, const Event &evt);

	void buildCommandList(FrameData* currentFrameData, u32 frameIndex);
	void submitCommandList(Graphics::CommandQueue* queue);

	void downloadToCpu();
};