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

#include "CopyManager.h"
#include "d3dx12.h"
#include "FrameData.h"

struct CopyManager::Entry
{
	enum class Type : u32 {Buffer, Texture};

	ID3D12Resource* src;
	u64 srcOffset;
	u64 size; 
	Type type;
	u32 srcStateBefore;
	ID3D12Resource* dst;
	u64 dstOffset;
	u32 dstStateBefore;
};

CopyManager::CopyManager()
	:m_currentCommandList(nullptr),
	m_commandLists(),
	m_buildList(0),
	m_entries()
{

}

CopyManager::~CopyManager()
{

}

bool CopyManager::initialize(ID3D12Device* device, FrameData* frameData, u32 frameCount)
{
	m_commandLists = std::make_unique<d3d::GraphicsCommandList[]>(frameCount);
	for (u32 i = 0; i < frameCount; ++i)
	{
		if (!m_commandLists[i].create(device, D3D12_COMMAND_LIST_TYPE_DIRECT, frameData[i].m_allocatorCopy.get()))
		{
			return false;
		}
	}

	return true;
}

void CopyManager::destroy()
{
}

void CopyManager::pushCopyBuffer(ID3D12Resource* src, u64 srcOffset, u32 srcStateBefore, u64 size, ID3D12Resource* dst, u64 dstOffset, u32 dstStateBefore)
{
	Entry entry;
	entry.dst = dst;
	entry.dstOffset = dstOffset;
	entry.dstStateBefore = dstStateBefore;
	entry.type = Entry::Type::Buffer;
	entry.size = size;
	entry.src = src;
	entry.srcOffset = srcOffset;
	entry.srcStateBefore = srcStateBefore;

	m_entries.push_back(entry);
}

void CopyManager::pushCopyTexture(ID3D12Resource* src, u32 srcStateBefore, ID3D12Resource* dst, u32 dstStateBefore)
{
	Entry entry;
	entry.dst = dst;
	entry.dstOffset = 0;
	entry.dstStateBefore = dstStateBefore;
	entry.type = Entry::Type::Texture;
	entry.size = 0;
	entry.src = src;
	entry.srcOffset = 0;
	entry.srcStateBefore = srcStateBefore;

	m_entries.push_back(entry);
}

void CopyManager::buildCommandList(FrameData* currentFrameData, u32 frameIndex)
{
	auto allocator = currentFrameData->m_allocatorDownload.get();
	auto &commandList = m_commandLists[frameIndex];

	if (!m_entries.empty())
	{
		allocator->Reset();
		commandList->Reset(allocator, nullptr);

		for (auto &it : m_entries)
		{
			switch (it.type)
			{
			case Entry::Type::Buffer:
			{
				commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(it.src, (D3D12_RESOURCE_STATES)it.srcStateBefore, D3D12_RESOURCE_STATE_COPY_SOURCE));
				commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(it.dst, (D3D12_RESOURCE_STATES)it.dstStateBefore, D3D12_RESOURCE_STATE_COPY_DEST));

				commandList->CopyBufferRegion(it.dst, it.dstOffset, it.src, it.srcOffset, it.size);

				commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(it.dst, D3D12_RESOURCE_STATE_COPY_DEST, (D3D12_RESOURCE_STATES)it.dstStateBefore));
				commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(it.src, D3D12_RESOURCE_STATE_COPY_SOURCE, (D3D12_RESOURCE_STATES)it.srcStateBefore));
			}break;
			case Entry::Type::Texture:
			{
				commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(it.src, (D3D12_RESOURCE_STATES)it.srcStateBefore, D3D12_RESOURCE_STATE_COPY_SOURCE));
				commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(it.dst, (D3D12_RESOURCE_STATES)it.dstStateBefore, D3D12_RESOURCE_STATE_COPY_DEST));

				auto locationSrc = CD3DX12_TEXTURE_COPY_LOCATION(it.src, 0);
				auto locationDst = CD3DX12_TEXTURE_COPY_LOCATION(it.dst, 0);
				commandList->CopyTextureRegion(&locationDst, 0, 0, 0, &locationSrc, nullptr);

				commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(it.dst, D3D12_RESOURCE_STATE_COPY_DEST, (D3D12_RESOURCE_STATES)it.dstStateBefore));
				commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(it.src, D3D12_RESOURCE_STATE_COPY_SOURCE, (D3D12_RESOURCE_STATES)it.srcStateBefore));
			}break;
			default:
				break;
			}
		}

		commandList->Close();
		m_entries.clear();

		m_currentCommandList = &commandList;
		m_buildList = 1;
	}
}

void CopyManager::submitList(Graphics::CommandQueue* queue)
{
	if (m_buildList != 0)
	{
		queue->pushCommandList(m_currentCommandList);
		m_buildList = 0;
	}
}