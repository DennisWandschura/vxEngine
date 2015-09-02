#pragma once

#include "RenderPass.h"
#include "DescriptorHeap.h"
#include "CommandList.h"

class RenderPassVoxelize : public RenderPass
{
	d3d::GraphicsCommandList m_commandList;
	ID3D12CommandAllocator* m_cmdAlloc;
	d3d::DescriptorHeap m_descriptorHeap;
	d3d::DescriptorHeap m_descriptorHeapClear;
	d3d::Object<ID3D12CommandSignature> m_commandSignature;
	u32 m_drawCount;
	u32 m_countOffset;

	bool loadShaders();
	bool createRootSignature(ID3D12Device* device);
	bool createPipelineState(ID3D12Device* device);
	bool createDescriptorHeap(ID3D12Device* device);
	void createViews(ID3D12Device* device);

	void uploadBufferData();

public:
	RenderPassVoxelize(ID3D12CommandAllocator* cmdAlloc, u32 countOffset);
	~RenderPassVoxelize();

	void getRequiredMemory(u64* heapSizeBuffer, u64* heapSizeTexture, u64* heapSizeRtDs, ID3D12Device* device) override;

	bool createData(ID3D12Device* device) override;

	bool initialize(ID3D12Device* device, void* p) override;
	void shutdown() override;

	void submitCommands(ID3D12CommandList** list, u32* index) override;

	void setDrawCount(u32 count) { m_drawCount = count; }
};