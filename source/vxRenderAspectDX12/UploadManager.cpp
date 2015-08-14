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
#include "UploadManager.h"
#include <d3d12.h>
#include "d3dx12.h"
#include "Device.h"

struct UploadManager::UploadTask
{
	UploadTaskType type;
	union
	{
		UploadTaskBuffer buffer;
		UploadTaskTexture texture;
	};

	UploadTask() {}
};

struct QueuedBufferTask
{
	ID3D12Resource* dst;
	u32 dstOffset;
	u32 size;
	u32 state;
};

struct UploadManager::QueuedUploadTask
{
	UploadTaskType type;
	std::unique_ptr<u8[]> m_data;
	union
	{
		UploadTaskTextureDesc texture;
		QueuedBufferTask buffer;
	};

	QueuedUploadTask() {}
};

struct UploadManager::UploadDesc
{
	D3D12_RANGE range;
	const u8* data;
	u32 size;
};

UploadManager::UploadManager()
	:m_tasks(),
	m_queue(),
	m_capacity(0),
	m_size(0),
	m_commandAllocator(),
	m_commandList(),
	m_uploadBuffer(),
	m_heap()
{

}

UploadManager::~UploadManager()
{

}

bool UploadManager::createHeap(d3d::Device* device)
{
	auto bufferSize = d3d::getAlignedSize(10u MBYTE, 64u KBYTE);
	if (!m_heap.createBufferHeap(bufferSize, D3D12_HEAP_TYPE_UPLOAD, device))
		return false;

	return true;
}
bool UploadManager::initialize(d3d::Device* device)
{
	if (!createHeap(device))
		return false;

	auto bufferSize = d3d::getAlignedSize(10u MBYTE, 64u KBYTE);
	if (!m_heap.createResourceBuffer(bufferSize, 0, D3D12_RESOURCE_STATE_GENERIC_READ, m_uploadBuffer.getAddressOf(), device))
		return false;

	auto hresult = device->getDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_commandAllocator.getAddressOf()));
	if (hresult != 0)
		return false;

	hresult = device->getDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.get(), nullptr, IID_PPV_ARGS(m_commandList.getAddressOf()));
	if (hresult != 0)
		return false;

	m_capacity = bufferSize;

	return true;
}

void UploadManager::uploadData(const UploadDesc &desc)
{
	u8* ptr = nullptr;
	auto hresult = m_uploadBuffer->Map(0, &desc.range, (void**)&ptr);
	ptr = ptr + desc.range.Begin;
	memcpy(ptr, desc.data, desc.size);
	m_uploadBuffer->Unmap(0, &desc.range);
}

void UploadManager::pushTaskBuffer(ID3D12Resource* dstBuffer, u32 offset, u32 dstOffset, u32 size, u32 state)
{
	UploadTask job;
	job.type = UploadTaskType::Buffer;
	job.buffer.dst = dstBuffer;
	job.buffer.srcOffset = offset;
	job.buffer.dstOffset = dstOffset;
	job.buffer.size = size;
	job.buffer.state = state;

	m_tasks.push_back(job);
}

void UploadManager::pushTaskTexture(const UploadTaskTextureDesc &desc, u32 srcOffset)
{
	UploadTask job;
	job.type = UploadTaskType::Texture;
	job.texture.dst = desc.dst;
	job.texture.dim = desc.dim;
	job.texture.format = desc.format;
	job.texture.rowPitch = desc.rowPitch;
	job.texture.srcOffset = srcOffset;
	job.texture.dstOffset = desc.slice;
	job.texture.state = desc.state;
	m_tasks.push_back(job);
}

bool UploadManager::tryUploadBuffer(const u8* data, ID3D12Resource* dstBuffer, u32 dstOffset, u32 size, u32 state)
{
	auto offset = m_size;
	auto alignedSize = d3d::getAlignedSize(size, 256);
	auto newSize = m_size + alignedSize;
	if (newSize >= m_capacity)
	{
		return false;
	}

	UploadDesc desc;
	desc.range.Begin = offset;
	desc.range.End = offset + alignedSize;
	desc.data = data;
	desc.size = size;

	uploadData(desc);
	pushTaskBuffer(dstBuffer, offset, dstOffset, size, state);

	m_size = newSize;

	return true;
}

void UploadManager::pushUploadBuffer(const u8* data, ID3D12Resource* dstBuffer, u32 dstOffset, u32 size, u32 state)
{
	if (!tryUploadBuffer(data, dstBuffer, dstOffset, size, state))
	{
		QueuedUploadTask queuedTask;
		queuedTask.type = UploadTaskType::Buffer;
		queuedTask.m_data = std::make_unique<u8[]>(size);
		queuedTask.buffer.dst = dstBuffer;
		queuedTask.buffer.state = state;
		queuedTask.buffer.dstOffset = dstOffset;
		queuedTask.buffer.size = size;

		memcpy(queuedTask.m_data.get(), data, size);

		m_queue.push_back(std::move(queuedTask));
	}
}

bool UploadManager::tryUploadTexture(const UploadTaskTextureDesc &desc)
{
	auto offset = d3d::getAlignedSize(m_size, 64 KBYTE);
	auto alignedSize = d3d::getAlignedSize(desc.dataSize, 256);

	auto newSize = offset + alignedSize;
	if (newSize >= m_capacity)
	{
		return false;
	}

	UploadDesc uploadDesc;
	uploadDesc.range.Begin = offset;
	uploadDesc.range.End = offset + alignedSize;
	uploadDesc.data = desc.data;
	uploadDesc.size = desc.dataSize;
	uploadData(uploadDesc);

	pushTaskTexture(desc, offset);

	m_size = newSize;
}

void UploadManager::pushUploadTexture(const UploadTaskTextureDesc &desc)
{
	if (!tryUploadTexture(desc))
	{
		QueuedUploadTask queuedTask;
		queuedTask.type = UploadTaskType::Texture;
		queuedTask.m_data = std::make_unique<u8[]>(desc.dataSize);
		queuedTask.texture = desc;
		queuedTask.texture.data = queuedTask.m_data.get();

		memcpy(queuedTask.m_data.get(), desc.data, desc.dataSize);

		m_queue.push_back(std::move(queuedTask));
	}
}

void UploadManager::processQueue()
{
	if (m_queue.empty())
		return;

	std::vector<QueuedUploadTask> newQueue;
	newQueue.reserve(m_queue.size());

	for (auto &it : m_queue)
	{
		switch (it.type)
		{
		case UploadTaskType::Buffer:
		{
			if (!tryUploadBuffer(it.m_data.get(), it.buffer.dst, it.buffer.dstOffset, it.buffer.size, it.buffer.state))
			{
				newQueue.push_back(std::move(it));
			}
		}break;
		case UploadTaskType::Texture:
		{
			if (!tryUploadTexture(it.texture))
			{
				newQueue.push_back(std::move(it));
			}

		}break;
		default:
			break;
		}
	}

	newQueue.swap(m_queue);
}

void UploadManager::processTasks()
{
	for (auto &it : m_tasks)
	{
		switch (it.type)
		{
		case UploadTaskType::Buffer:
		{
			m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(it.buffer.dst, (D3D12_RESOURCE_STATES)it.buffer.state, D3D12_RESOURCE_STATE_COPY_DEST));
			m_commandList->CopyBufferRegion(it.buffer.dst, it.buffer.dstOffset, m_uploadBuffer.get(), it.buffer.srcOffset, it.buffer.size);
			m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(it.buffer.dst, D3D12_RESOURCE_STATE_COPY_DEST, (D3D12_RESOURCE_STATES)it.buffer.state));
		}break;
		case UploadTaskType::Texture:
		{
			m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(it.texture.dst, (D3D12_RESOURCE_STATES)it.texture.state, D3D12_RESOURCE_STATE_COPY_DEST));

			CD3DX12_TEXTURE_COPY_LOCATION dstLoc(it.texture.dst, it.texture.dstOffset);

			D3D12_TEXTURE_COPY_LOCATION srcLoc;
			srcLoc.pResource = m_uploadBuffer.get();
			srcLoc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
			srcLoc.PlacedFootprint.Offset = it.texture.srcOffset;
			srcLoc.PlacedFootprint.Footprint.Depth = 1;
			srcLoc.PlacedFootprint.Footprint.Format = (DXGI_FORMAT)it.texture.format;
			srcLoc.PlacedFootprint.Footprint.Width = it.texture.dim.x;
			srcLoc.PlacedFootprint.Footprint.Height = it.texture.dim.y;
			srcLoc.PlacedFootprint.Footprint.RowPitch = it.texture.rowPitch;

			m_commandList->CopyTextureRegion(&dstLoc, 0, 0, 0, &srcLoc, nullptr);

			m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(it.texture.dst, D3D12_RESOURCE_STATE_COPY_DEST, (D3D12_RESOURCE_STATES)it.texture.state));
		}break;
		}
	}
	m_tasks.clear();
}

ID3D12GraphicsCommandList* UploadManager::update()
{
	processQueue();

	m_commandAllocator->Reset();
	m_commandList->Reset(m_commandAllocator.get(), nullptr);

	processTasks();

	auto hresult = m_commandList->Close();
	if (hresult != 0)
	{
		printf("error upload\n");
		VX_ASSERT(false);
	}

	m_size = 0;

	return m_commandList.get();
}