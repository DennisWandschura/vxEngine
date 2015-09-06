#pragma once

struct ID3D12QueryHeap;

namespace d3d
{
	class GraphicsCommandList;
	class ResourceManager;
}

#include "d3d.h"

class GpuProfiler
{
	ID3D12Resource* m_buffer;
	d3d::Object<ID3D12QueryHeap> m_queryHeap;
	s64 m_currentFrame;
	u32 m_count[3];
	u32 m_countPerFrame;

public:
	GpuProfiler();
	~GpuProfiler();

	void getRequiredMemory(u32 maxQueries, u64* bufferHeapSize);

	bool initialize(u32 maxQueries, d3d::ResourceManager* resourceManager, ID3D12Device* device);
	void shutdown();

	void frame(d3d::GraphicsCommandList* cmdList);

	void query(d3d::GraphicsCommandList* cmdList);
};