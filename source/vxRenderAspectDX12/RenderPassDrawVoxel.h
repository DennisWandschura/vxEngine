#pragma once

namespace d3d
{
	class CommandAllocator;
}

#include "RenderPass.h"
#include "CommandList.h"
#include "DescriptorHeap.h"

class RenderPassDrawVoxel : public RenderPass
{
	d3d::GraphicsCommandList m_commandList;
	d3d::CommandAllocator* m_cmdAlloc;
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
	explicit RenderPassDrawVoxel(d3d::CommandAllocator* cmdAlloc);
	~RenderPassDrawVoxel();

	void getRequiredMemory(u64* heapSizeBuffer, u64* heapSizeTexture, u64* heapSizeRtDs, ID3D12Device* device) override;

	bool createData(ID3D12Device* device) override;

	bool initialize(ID3D12Device* device, void* p) override;
	void shutdown() override;

	void submitCommands(Graphics::CommandQueue* queue) override;
};