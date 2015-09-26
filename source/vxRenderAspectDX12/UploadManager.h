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

struct ID3D12Resource;
struct ID3D12CommandAllocator;
class Event;

namespace d3d
{
	class Device;
	class CommandQueue;
}

#include <vxLib/types.h>
#include "d3d.h"
#include "Heap.h"
#include <vector>
#include <vxLib/Allocator/StackAllocator.h>
#include <vxLib/math/Vector.h>
#include <mutex>
#include "CommandList.h"
#include "CommandAllocator.h"

struct UploadTaskTexture
{
	ID3D12Resource* dst;
	vx::uint2 dim;
	u32 format;
	u32 rowPitch;
	u32 srcOffset;
	u32 dstOffset;
	u32 state;
};

struct UploadTaskTextureDesc
{
	ID3D12Resource* dst;
	const u8* data;
	vx::uint2 dim;
	u32 format;
	u32 rowPitch;
	u32 slice;
	u32 dataSize;
	u32 state;
};

struct UploadTaskBuffer
{
	ID3D12Resource* dst;
	u32 srcOffset;
	u32 dstOffset;
	u32 size;
	u32 state;
};

class UploadManager
{
	enum class UploadTaskType :u32 { Buffer, Texture };

	struct UploadTask;
	struct QueuedUploadTask;

	struct UploadDesc;

	std::mutex m_taskMutex;
	std::vector<UploadTask> m_tasks;
	std::vector<UploadTask> m_tasksBack;
	std::mutex m_queueMutex;
	std::vector<QueuedUploadTask> m_queue;
	u32 m_capacity;
	u32 m_size;
	d3d::CommandAllocator m_commandAllocator;
	d3d::GraphicsCommandList m_commandList;
	d3d::Object<ID3D12Resource> m_uploadBuffer;
	d3d::Heap m_heap;

	bool createHeap(ID3D12Device* device);

	void uploadData(const UploadDesc &desc);

	void pushQueueTask(QueuedUploadTask &&task);
	bool tryUploadBuffer(const u8* data, ID3D12Resource* dstBuffer, u32 dstOffset, u32 size, u32 state);
	void pushTaskBuffer(ID3D12Resource* dstBuffer, u32 offset, u32 dstOffset, u32 size, u32 state);

	bool tryUploadTexture(const UploadTaskTextureDesc &desc);
	void pushTaskTexture(const UploadTaskTextureDesc &desc, u32 srcOffset);

	void processQueue();
	void processTasks();

public:
	UploadManager();
	~UploadManager();

	bool initialize(ID3D12Device* device);
	void shutdown();

	template<typename T>
	void pushUploadBuffer(const T &data, ID3D12Resource* dstBuffer, u32 dstOffset, u32 state)
	{
		pushUploadBuffer((u8*)&data, dstBuffer, dstOffset, sizeof(T), state);
	}

	void pushUploadBuffer(const u8* data, ID3D12Resource* dstBuffer, u32 dstOffset, u32 size, u32 state);
	void pushUploadBuffer(const u8* data, ID3D12Resource* dstBuffer, u32 dstOffset, u32 size, u32 state, const Event &evt);
	void pushUploadTexture(const UploadTaskTextureDesc &desc);

	void buildCommandList();
	void submitCommandList(d3d::CommandQueue* queue);
};