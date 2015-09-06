#pragma once

#include "RenderPass.h"
#include "CommandList.h"
#include "DescriptorHeap.h"

class RenderPassRadiosity : public RenderPass
{
	ID3D12CommandAllocator* m_allocator;
	d3d::GraphicsCommandList m_commandList;
	d3d::DescriptorHeap m_rtvHeap;
	d3d::DescriptorHeap m_srvHeap;
	d3d::Object<ID3D12RootSignature> m_rootSignatureGather;
	d3d::PipelineState m_pipelineStateGather;

	bool loadShaders();
	bool createRootSignature(ID3D12Device* device);
	bool createRootSignatureGather(ID3D12Device* device);
	bool createPipelineState(ID3D12Device* device);
	bool createPipelineStateGather(ID3D12Device* device);
	bool createRtv(ID3D12Device* device);
	bool createSrv(ID3D12Device* device);
	bool createCommandList(ID3D12Device* device);

public:
	explicit RenderPassRadiosity(ID3D12CommandAllocator* alloc);
	~RenderPassRadiosity();

	void getRequiredMemory(u64* heapSizeBuffer, u64* heapSizeTexture, u64* heapSizeRtDs, ID3D12Device* device) override;

	bool createData(ID3D12Device* device) override;

	bool initialize(ID3D12Device* device, void* p) override;
	void shutdown() override;

	void submitCommands(ID3D12CommandList** list, u32* index) override;
};