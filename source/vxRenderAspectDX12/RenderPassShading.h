#pragma once

#include "RenderPass.h"
#include "DescriptorHeap.h"
#include "CommandList.h"

class RenderPassShading : public RenderPass
{
	d3d::GraphicsCommandList m_commandList;
	ID3D12CommandAllocator* m_cmdAlloc;
	d3d::DescriptorHeap m_heapSrv;
	d3d::DescriptorHeap m_heapRtv;
	vx::uint2 m_resolution;

	bool createTexture(ID3D12Device* device);
	bool loadShaders();
	bool createRootSignature(ID3D12Device* device);
	bool createPipelineState(ID3D12Device* device);
	bool createSrv(ID3D12Device* device);
	bool createRtv(ID3D12Device* device);

public:
	RenderPassShading(const vx::uint2 &resolution, ID3D12CommandAllocator* cmdAlloc);
	~RenderPassShading();

	void getRequiredMemory(u64* heapSizeBuffer, u64* heapSizeTexture, u64* heapSizeRtDs, ID3D12Device* device) override;

	bool initialize(ID3D12Device* device, void* p)override;
	void shutdown() override;

	ID3D12CommandList* submitCommands() override;
};