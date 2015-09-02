#pragma once

#include "RenderPass.h"
#include "CommandList.h"
#include "DescriptorHeap.h"

class RenderPassDrawVoxel : public RenderPass
{
	d3d::GraphicsCommandList m_commandList;
	ID3D12CommandAllocator* m_cmdAlloc;
	d3d::DescriptorHeap m_descriptorHeapSrv;
	d3d::Object<ID3D12DescriptorHeap> m_heapRtv;
	d3d::Object<ID3D12DescriptorHeap> m_heapDsv;

	bool loadShaders();
	bool createRootSignature(ID3D12Device* device);
	bool createPipelineState(ID3D12Device* device);
	bool createDescriptorHeap(ID3D12Device* device);
	void createViews(ID3D12Device* device);
	void createDepthRenderTarget(ID3D12Device* device);

public:
	explicit RenderPassDrawVoxel(ID3D12CommandAllocator* cmdAlloc);
	~RenderPassDrawVoxel();

	void getRequiredMemory(u64* heapSizeBuffer, u64* heapSizeTexture, u64* heapSizeRtDs, ID3D12Device* device) override;

	bool createData(ID3D12Device* device) override;

	bool initialize(ID3D12Device* device, void* p) override;
	void shutdown() override;

	void submitCommands(ID3D12CommandList** list, u32* index) override;
};