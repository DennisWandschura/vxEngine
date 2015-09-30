#pragma once

namespace d3d
{
	class CommandAllocator;
}

#include "RenderPass.h"
#include "DescriptorHeap.h"
#include "CommandList.h"

class RenderPassCreateVpl : public RenderPass
{
	d3d::GraphicsCommandList m_commandList;
	d3d::CommandAllocator* m_allocator;
	d3d::DescriptorHeap m_srvHeap;
	d3d::DescriptorHeap m_uavHeap;

	bool createRootSignature(ID3D12Device* device);
	bool createPipelineStage(ID3D12Device* device);
	bool createDescriptorHeap(ID3D12Device* device);

public:
	explicit RenderPassCreateVpl(d3d::CommandAllocator* allocator);
	~RenderPassCreateVpl();

	void getRequiredMemory(u64* heapSizeBuffer, u32* bufferCount, u64* heapSizeTexture, u32* textureCount, u64* heapSizeRtDs, u32* rtDsCount, ID3D12Device* device) override;

	bool createData(ID3D12Device* device) override;

	bool initialize(ID3D12Device* device, void* p) override;
	void shutdown() override;

	void buildCommands() override;
	void submitCommands(Graphics::CommandQueue* queue) override;
};