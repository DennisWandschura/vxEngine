#pragma once

namespace d3d
{
	class CommandAllocator;
}

#include "RenderPass.h"
#include "DescriptorHeap.h"
#include "CommandList.h"

class RenderPassVoxelMip : public RenderPass 
{
	d3d::GraphicsCommandList m_commandList;
	d3d::CommandAllocator* m_cmdAlloc;
	d3d::DescriptorHeap m_uavHeap;

	bool loadShaders();
	bool createRootSignature(ID3D12Device* device);
	bool createPipelineState(ID3D12Device* device);
	void createViews(ID3D12Device* device);

public:
	explicit RenderPassVoxelMip(d3d::CommandAllocator* cmdAlloc);
	~RenderPassVoxelMip();

	void getRequiredMemory(u64* heapSizeBuffer, u64* heapSizeTexture, u64* heapSizeRtDs, ID3D12Device* device) override;

	bool createData(ID3D12Device* device) override;

	bool initialize(ID3D12Device* device, void* p) override;
	void shutdown() override;

	void submitCommands(Graphics::CommandQueue* queue) override;
};