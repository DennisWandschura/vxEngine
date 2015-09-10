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
#include <atomic>
#include "CommandQueue.h"
#include <vxEngineLib/Event.h>

namespace UploadManagerCpp
{
	const u64 g_bufferSize = d3d::AlignedSize<20u MBYTE, 64u KBYTE>::size;;
}

struct UploadManager::UploadTask
{
	UploadTaskType type;
	union
	{
		UploadTaskBuffer buffer;
		UploadTaskTexture texture;
	};
	Event evt;

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
	:m_taskMutex(),
	m_tasks(),
	m_queueMutex(),
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

bool UploadManager::createHeap(ID3D12Device* device)
{
	if (!m_heap.createBufferHeap(UploadManagerCpp::g_bufferSize, D3D12_HEAP_TYPE_UPLOAD, device))
		return false;

	m_heap.setName(L"UploadManagerHeap");

	return true;
}
bool UploadManager::initialize(ID3D12Device* device)
{
	if (!createHeap(device))
		return false;

	d3d::HeapCreateBufferResourceDesc desc;
	desc.size = UploadManagerCpp::g_bufferSize;
	desc.state = D3D12_RESOURCE_STATE_GENERIC_READ;
	desc.resource = m_uploadBuffer.getAddressOf();
	desc.flags = D3D12_RESOURCE_FLAG_NONE;
	if (!m_heap.createResourceBuffer(desc))
		return false;

	auto hresult = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_commandAllocator.getAddressOf()));
	if (hresult != 0)
		return false;

	m_commandAllocator->SetName(L"UploadManagerCommandAllocator");

	if (!m_commandList.create(device, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.get(), nullptr))
		return false;

	m_commandList->SetName(L"UploadManagerCommandList");

	m_capacity = UploadManagerCpp::g_bufferSize;

	return true;
}

void UploadManager::shutdown()
{
	m_capacity = 0;

	m_commandList.destroy();
	m_commandAllocator.destroy();
	m_uploadBuffer.destroy();
	m_heap.destroy();
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

	std::lock_guard<std::mutex> lock(m_taskMutex);
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

	std::lock_guard<std::mutex> lock(m_taskMutex);
	m_tasks.push_back(job);
}

bool UploadManager::tryUploadBuffer(const u8* data, ID3D12Resource* dstBuffer, u32 dstOffset, u32 size, u32 state)
{
	auto offset = d3d::getAlignedSize(m_size, 256u);
	auto alignedSize = d3d::getAlignedSize(size, 256u);
	auto newSize = offset + alignedSize;
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

void UploadManager::pushQueueTask(QueuedUploadTask &&task)
{
	std::lock_guard<std::mutex> lock(m_queueMutex);
	m_queue.push_back(std::move(task));
}

void UploadManager::pushUploadBuffer(const u8* data, ID3D12Resource* dstBuffer, u32 dstOffset, u32 size, u32 state)
{
	Event emptyEvt;

	pushUploadBuffer(data, dstBuffer, dstOffset, size, state, emptyEvt);
}

void UploadManager::pushUploadBuffer(const u8* data, ID3D12Resource* dstBuffer, u32 dstOffset, u32 size, u32 state, const Event &evt)
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

		pushQueueTask(std::move(queuedTask));
	}
}

bool UploadManager::tryUploadTexture(const UploadTaskTextureDesc &desc)
{
	auto offset = d3d::getAlignedSize(m_size, 512u);
	auto alignedSize = d3d::getAlignedSize(desc.dataSize, 256u);

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

	return true;
}

void UploadManager::pushUploadTexture(const UploadTaskTextureDesc &desc)
{
	VX_ASSERT(desc.dst != nullptr);

	if (!tryUploadTexture(desc))
	{
		QueuedUploadTask queuedTask;
		queuedTask.type = UploadTaskType::Texture;
		queuedTask.m_data = std::make_unique<u8[]>(desc.dataSize);
		queuedTask.texture = desc;
		queuedTask.texture.data = queuedTask.m_data.get();

		memcpy(queuedTask.m_data.get(), desc.data, desc.dataSize);

		pushQueueTask(std::move(queuedTask));
	}
}

void UploadManager::processQueue()
{
	std::lock_guard<std::mutex> lock(m_queueMutex);

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
	{
		std::unique_lock<std::mutex> lock(m_taskMutex);
		m_tasksBack.swap(m_tasks);
		lock.unlock();
	}

	for (auto &it : m_tasksBack)
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

			D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
			footprint.Offset = it.texture.srcOffset;
			footprint.Footprint.Depth = 1;
			footprint.Footprint.Format = (DXGI_FORMAT)it.texture.format;
			footprint.Footprint.Height = it.texture.dim.y;
			footprint.Footprint.RowPitch = it.texture.rowPitch;
			footprint.Footprint.Width = it.texture.dim.x;

			CD3DX12_TEXTURE_COPY_LOCATION Dst(it.texture.dst, it.texture.dstOffset);
			CD3DX12_TEXTURE_COPY_LOCATION Src(m_uploadBuffer.get(), footprint);
			m_commandList->CopyTextureRegion(&Dst, 0, 0, 0, &Src, nullptr);

			m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(it.texture.dst, D3D12_RESOURCE_STATE_COPY_DEST, (D3D12_RESOURCE_STATES)it.texture.state));
		}break;
		}

		if (it.evt.isValid())
		{
			it.evt.setStatus(EventStatus::Complete);
		}
	}

	m_tasksBack.clear();
}

void UploadManager::submitCommandList(d3d::CommandQueue* queue)
{
	processQueue();

	auto hr = m_commandAllocator->Reset();
	if (hr != 0)
	{
		puts("errror");
	}

	hr = m_commandList->Reset(m_commandAllocator.get(), nullptr);
	if (hr != 0)
	{
		puts("errror");
	}

	processTasks();

	auto hresult = m_commandList->Close();
	if (hresult != 0)
	{
		//printf("error upload\n");
		VX_ASSERT(false);
	}

	m_size = 0;

	queue->pushCommandList(&m_commandList);
}