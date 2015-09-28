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

struct ID3D12QueryHeap;
struct ID3D12CommandQueue;
class RenderAspect;

namespace d3d
{
	class GraphicsCommandList;
	class ResourceManager;
	class Resource;
}

namespace Graphics
{
	class CommandQueue;
}

#include <vxLib/math/Vector.h>
#include "d3d.h"
#include <memory>
#include <vxEngineLib/Event.h>
#include "CommandAllocator.h"
#include "CommandList.h"
#include "Heap.h"
#include <vxLib/Container/sorted_vector.h>
#include <vxLib/StringID.h>

class GpuProfiler
{
	struct Entry;

	struct Text;

	struct FrameData
	{
		std::unique_ptr<Text[]> m_entries;
		std::unique_ptr<u64[]> m_times;
		u32 m_count;
	};

	d3d::CommandAllocator m_allocator;
	d3d::GraphicsCommandList m_commandList;
	FrameData m_data[3];
	std::unique_ptr<Entry[]> m_entries;
	u32 m_entryCount;
	vx::sorted_vector<vx::StringID, u32> m_sortedEntries;
	d3d::Object<ID3D12Resource> m_dlBuffer;
	u64 m_frequency;
	vx::float2 m_position;
	d3d::Object<ID3D12QueryHeap> m_queryHeap;
	s64 m_currentFrame;
	u32 m_countPerFrame;
	u32 m_buildCommands;
	d3d::Heap m_dlHeap;

public:
	GpuProfiler();
	~GpuProfiler();

	void getRequiredMemory(u32 maxQueries, u64* bufferHeapSize);

	bool initialize(u32 maxQueries, d3d::ResourceManager* resourceManager, ID3D12Device* device, ID3D12CommandQueue* cmdQueue, const vx::float2 &position);
	void shutdown();

	void frame(d3d::ResourceManager* resourceManager);
	void submitCommandList(Graphics::CommandQueue* queue);

	void queryBegin(const char* text, d3d::GraphicsCommandList* cmdList);
	void queryEnd(d3d::GraphicsCommandList* cmdList);

	void update(RenderAspect* renderAspect);
};