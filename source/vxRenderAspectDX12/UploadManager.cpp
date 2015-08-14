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
	ID3D12Resource* dst;
	u32 srcOffset;
	u32 dstOffset;
	u32 size;
	u32 state;
};

struct UploadManager::QueuedUploadTask
{
	ID3D12Resource* dst;
	std::unique_ptr<u8[]> m_data;
	u32 dstOffset;
	u32 size;
	u32 state;
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

bool UploadManager::initialize(d3d::Device* device)
{
	auto bufferSize = d3d::getAlignedSize(10u MBYTE, 64u KBYTE);
	if (!m_heap.createBufferHeap(bufferSize, D3D12_HEAP_TYPE_UPLOAD, device))
		return false;

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

bool UploadManager::tryUpload(const u8* data, ID3D12Resource* dstBuffer, u32 dstOffset, u32 size, u32 state)
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

	UploadTask job;
	job.dst = dstBuffer;
	job.srcOffset = offset;
	job.dstOffset = dstOffset;
	job.size = size;
	job.state = state;

	m_tasks.push_back(job);
	m_size = newSize;

	return true;
}

void UploadManager::pushUpload(const u8* data, ID3D12Resource* dstBuffer, u32 dstOffset, u32 size, u32 state)
{
	if (!tryUpload(data, dstBuffer, dstOffset, size, state))
	{
		QueuedUploadTask queuedTask;
		queuedTask.dst = dstBuffer;
		queuedTask.m_data = std::make_unique<u8[]>(size);
		queuedTask.state = state;
		queuedTask.dstOffset = dstOffset;
		queuedTask.size = size;

		memcpy(queuedTask.m_data.get(), data, size);

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
		if (!tryUpload(it.m_data.get(), it.dst, it.dstOffset, it.size, it.state))
		{
			newQueue.push_back(std::move(it));
		}
	}

	newQueue.swap(m_queue);
}

void UploadManager::processTasks()
{
	for (auto &it : m_tasks)
	{
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(it.dst, (D3D12_RESOURCE_STATES)it.state, D3D12_RESOURCE_STATE_COPY_DEST));

		m_commandList->CopyBufferRegion(it.dst, it.dstOffset, m_uploadBuffer.get(), it.srcOffset, it.size);

		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(it.dst, D3D12_RESOURCE_STATE_COPY_DEST, (D3D12_RESOURCE_STATES)it.state));
	}
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
		printf("erro upload\n");
	}

	m_size = 0;

	return m_commandList.get();
}